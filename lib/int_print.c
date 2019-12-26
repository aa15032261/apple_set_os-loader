#include "../include/int_print.h"
#include "../include/int_mem.h"


#define PRINT_STRING_LEN            200
#define PRINT_ITEM_BUFFER_LEN       100

typedef struct {
    BOOLEAN             Ascii;
    UINTN               Index;
    union {
        CONST CHAR16    *pw;
        CONST CHAR8     *pc;
    } un;
} POINTER;

#define pw	un.pw
#define pc	un.pc

typedef struct _pitem {

    POINTER     Item;
    CHAR16      Scratch[PRINT_ITEM_BUFFER_LEN];
    UINTN       Width;
    UINTN       FieldWidth;
    UINTN       *WidthParse;
    CHAR16      Pad;
    BOOLEAN     PadBefore;
    BOOLEAN     Comma;
    BOOLEAN     Long;
} PRINT_ITEM;


typedef struct _pstate {
    // Input
    POINTER     fmt;
    va_list     args;

    // Output
    CHAR16      *Buffer;
    CHAR16      *End;
    CHAR16      *Pos;
    UINTN       Len;

    UINTN       Attr;
    UINTN       RestoreAttr;

    UINTN       AttrNorm;
    UINTN       AttrHighlight;
    UINTN       AttrError;

    INTN        (EFIAPI *Output)(VOID *context, CHAR16 *str);
    INTN        (EFIAPI *SetAttr)(VOID *context, UINTN attr);
    VOID        *Context;

    // Current item being formatted
    struct _pitem  *Item;
} PRINT_STATE;

//
// Internal fucntions
//

STATIC UINTN __INT_Print (
    IN PRINT_STATE     *ps
);

STATIC UINTN __INT_IPrint (
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN CONST CHAR16                     *fmt,
    IN CONST CHAR8                      *fmta,
    IN va_list                          args
);


STATIC VOID __INT_PFLUSH (
    IN OUT PRINT_STATE     *ps
);

STATIC VOID __INT_PPUTC (
    IN OUT PRINT_STATE     *ps,
    IN CHAR16              c
);

STATIC VOID __INT_PITEM (
    IN OUT PRINT_STATE  *ps
);

STATIC CHAR16 __INT_PGETC (
    IN POINTER      *p
);

STATIC VOID __INT_PSETATTR (
    IN OUT PRINT_STATE  *ps,
    IN UINTN             Attr
);

//
//
//

VOID _INT_Clear (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE    *Out
) {
    Out->ClearScreen(Out);
}

UINTN _INT_IPrint (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE    *Out,
    IN CONST CHAR16                    *fmt,
    ...
) {
    va_list     args;
    UINTN       back;

    va_start (args, fmt);
    back = __INT_IPrint ((UINTN) -1, (UINTN) -1, Out, fmt, NULL, args);
    va_end (args);
    return back;
}


UINTN _INT_IPrintAt (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN CONST CHAR16                     *fmt,
    ...
) {
    va_list     args;
    UINTN       back;

    va_start (args, fmt);
    back = __INT_IPrint (Column, Row, Out, fmt, NULL, args);
    va_end (args);
    return back;
}


UINTN __INT_IPrint (
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN CONST CHAR16                     *fmt,
    IN CONST CHAR8                      *fmta,
    IN va_list                          args
) {
    PRINT_STATE     ps;
    UINTN            back;

    _INT_memset(&ps, 0, sizeof(ps));
    ps.Context = Out;
    ps.Output  = (INTN (EFIAPI *)(VOID *, CHAR16 *)) Out->OutputString;
    ps.SetAttr = (INTN (EFIAPI *)(VOID *, UINTN))  Out->SetAttribute;
    ps.Attr = Out->Mode->Attribute;

    back = (ps.Attr >> 4) & 0xF;
    ps.AttrNorm = EFI_TEXT_ATTR(EFI_LIGHTGRAY, back);
    ps.AttrHighlight = EFI_TEXT_ATTR(EFI_WHITE, back);
    ps.AttrError = EFI_TEXT_ATTR(EFI_YELLOW, back);

    if (fmt) {
        ps.fmt.pw = fmt;
    } else {
        ps.fmt.Ascii = TRUE;
        ps.fmt.pc = fmta;
    }

    va_copy(ps.args, args);

    if (Column != (UINTN) -1) {
        uefi_call_wrapper(Out->SetCursorPosition, 3, Out, Column, Row);
    }

    back = __INT_Print (&ps);
    va_end(ps.args);
    return back;
}


STATIC VOID __INT_PFLUSH (
    IN OUT PRINT_STATE     *ps
) {
    *ps->Pos = 0;
    uefi_call_wrapper(ps->Output, 2, ps->Context, ps->Buffer);
    ps->Pos = ps->Buffer;
}

STATIC VOID __INT_PSETATTR (
    IN OUT PRINT_STATE  *ps,
    IN UINTN             Attr
) {
   __INT_PFLUSH (ps);

   ps->RestoreAttr = ps->Attr;
   if (ps->SetAttr) {
	uefi_call_wrapper(ps->SetAttr, 2, ps->Context, Attr);
   }

   ps->Attr = Attr;
}

STATIC VOID __INT_PPUTC (
    IN OUT PRINT_STATE     *ps,
    IN CHAR16              c
) {
    // if this is a newline, add a carraige return
    if (c == '\n') {
        __INT_PPUTC (ps, '\r');
    }

    *ps->Pos = c;
    ps->Pos += 1;
    ps->Len += 1;

    // if at the end of the buffer, flush it
    if (ps->Pos >= ps->End) {
        __INT_PFLUSH(ps);
    }
}


STATIC CHAR16 __INT_PGETC (
    IN POINTER      *p
) {
    CHAR16      c;

    c = p->Ascii ? p->pc[p->Index] : p->pw[p->Index];
    p->Index += 1;

    return  c;
}


STATIC VOID __INT_PITEM (
    IN OUT PRINT_STATE  *ps
) {
    UINTN               Len, i;
    PRINT_ITEM          *Item;
    CHAR16              c;

    // Get the length of the item
    Item = ps->Item;
    Item->Item.Index = 0;
    while (Item->Item.Index < Item->FieldWidth) {
        c = __INT_PGETC(&Item->Item);
        if (!c) {
            Item->Item.Index -= 1;
            break;
        }
    }
    Len = Item->Item.Index;

    // if there is no item field width, use the items width
    if (Item->FieldWidth == (UINTN) -1) {
        Item->FieldWidth = Len;
    }

    // if item is larger then width, update width
    if (Len > Item->Width) {
        Item->Width = Len;
    }


    // if pad field before, add pad char
    if (Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            __INT_PPUTC (ps, ' ');
        }
    }

    // pad item
    for (i=Len; i < Item->Width; i++) {
        __INT_PPUTC (ps, Item->Pad);
    }

    // add the item
    Item->Item.Index=0;
    while (Item->Item.Index < Len) {
        __INT_PPUTC (ps, __INT_PGETC(&Item->Item));
    }

    // If pad at the end, add pad char
    if (!Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            __INT_PPUTC (ps, ' ');
        }
    }
}


STATIC UINTN __INT_Print (
    IN PRINT_STATE     *ps
) {
    CHAR16          c;
    UINTN           Attr;
    PRINT_ITEM      Item;
    CHAR16          Buffer[PRINT_STRING_LEN];

    ps->Len = 0;
    ps->Buffer = Buffer;
    ps->Pos = Buffer;
    ps->End = Buffer + PRINT_STRING_LEN - 1;
    ps->Item = &Item;

    ps->fmt.Index = 0;
    while ((c = __INT_PGETC(&ps->fmt))) {

        if (c != '%') {
            __INT_PPUTC ( ps, c );
            continue;
        }

        // setup for new item
        Item.FieldWidth = (UINTN) -1;
        Item.Width = 0;
        Item.WidthParse = &Item.Width;
        Item.Pad = ' ';
        Item.PadBefore = TRUE;
        Item.Comma = FALSE;
        Item.Long = FALSE;
        Item.Item.Ascii = FALSE;
        Item.Item.pw = NULL;
        ps->RestoreAttr = 0;
        Attr = 0;

        while ((c = __INT_PGETC(&ps->fmt))) {

            switch (c) {

                case '%':
                {
                    Item.Scratch[0] = '%';
                    Item.Scratch[1] = 0;
                    Item.Item.pw = Item.Scratch;
                    break;
                }
                case '0':
                    Item.Pad = '0';
                    break;

                case '-':
                    Item.PadBefore = FALSE;
                    break;

                case ',':
                    Item.Comma = TRUE;
                    break;

                case '.':
                    Item.WidthParse = &Item.FieldWidth;
                    break;

                case '*':
                    *Item.WidthParse = va_arg(ps->args, UINTN);
                    break;

                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    *Item.WidthParse = 0;
                    do {
                        *Item.WidthParse = *Item.WidthParse * 10 + c - '0';
                        c = __INT_PGETC(&ps->fmt);
                    } while (c >= '0'  &&  c <= '9') ;
                    ps->fmt.Index -= 1;
                    break;

                case 'a':
                    Item.Item.pc = va_arg(ps->args, CHAR8 *);
                    Item.Item.Ascii = TRUE;
                    if (!Item.Item.pc) {
                        Item.Item.pc = (CHAR8 *)"(null)";
                    }
                    break;

                case 's':
                    Item.Item.pw = va_arg(ps->args, CHAR16 *);
                    if (!Item.Item.pw) {
                        Item.Item.pw = L"(null)";
                    }
                    break;

                case 'c':
                    Item.Scratch[0] = (CHAR16) va_arg(ps->args, UINTN);
                    Item.Scratch[1] = 0;
                    Item.Item.pw = Item.Scratch;
                    break;

                case 'l':
                    Item.Long = TRUE;
                    break;

                case 'X':
                    Item.Width = Item.Long ? 16 : 8;
                    Item.Pad = '0';
#if __GNUC__ >= 7
            __attribute__ ((fallthrough));
#endif
                case 'x':
                    ValueToHex (
                        Item.Scratch,
                        Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINT32)
                        );
                    Item.Item.pw = Item.Scratch;

                    break;


                case 'g':
                    GuidToString (Item.Scratch, va_arg(ps->args, EFI_GUID *));
                    Item.Item.pw = Item.Scratch;
                    break;

                case 'u':
                {
                    ValueToString (
                        Item.Scratch,
                        Item.Comma,
                        Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINT32)
                    );
                    Item.Item.pw = Item.Scratch;
                    break;
                }
                case 'd':
                    ValueToString (
                        Item.Scratch,
                        Item.Comma,
                        Item.Long ? va_arg(ps->args, INT64) : va_arg(ps->args, INT32)
                    );
                    Item.Item.pw = Item.Scratch;
                    break;
                case 'f':
                    FloatToString (
                        Item.Scratch,
                        Item.Comma,
                        va_arg(ps->args, double)
                    );
                    Item.Item.pw = Item.Scratch;
                    break;

                case 't':
                    TimeToString (Item.Scratch, va_arg(ps->args, EFI_TIME *));
                    Item.Item.pw = Item.Scratch;
                    break;

                case 'r':
                    StatusToString (Item.Scratch, va_arg(ps->args, EFI_STATUS));
                    Item.Item.pw = Item.Scratch;
                    break;

                case 'n':
                    __INT_PSETATTR(ps, ps->AttrNorm);
                    break;

                case 'h':
                    __INT_PSETATTR(ps, ps->AttrHighlight);
                    break;

                case 'e':
                    __INT_PSETATTR(ps, ps->AttrError);
                    break;

                case 'N':
                    Attr = ps->AttrNorm;
                    break;

                case 'H':
                    Attr = ps->AttrHighlight;
                    break;

                case 'E':
                    Attr = ps->AttrError;
                    break;

                default:
                    Item.Scratch[0] = '?';
                    Item.Scratch[1] = 0;
                    Item.Item.pw = Item.Scratch;
                    break;
            }

            // if we have an Item
            if (Item.Item.pw) {
                __INT_PITEM (ps);
                break;
            }

            // if we have an Attr set
            if (Attr) {
                __INT_PSETATTR(ps, Attr);
                ps->RestoreAttr = 0;
                break;
            }
        }

        if (ps->RestoreAttr) {
            __INT_PSETATTR(ps, ps->RestoreAttr);
        }
    }

    // Flush buffer
    __INT_PFLUSH (ps);
    return ps->Len;
}

STATIC CHAR8 Hex[] = {'0','1','2','3','4','5','6','7', '8','9','A','B','C','D','E','F'};

VOID ValueToHex (
    IN CHAR16   *Buffer,
    IN UINT64   v
) {
    CHAR8           str[30], *p1;
    CHAR16          *p2;

    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }

    p1 = str;
    p2 = Buffer;

    while (v) {
        // Without the cast, the MSVC compiler may insert a reference to __allmull
        *(p1++) = Hex[(UINTN)(v & 0xf)];
        v = v >> 4;
    }

    while (p1 != str) {
        *(p2++) = *(--p1);
    }
    *p2 = 0;
}


VOID ValueToString (
    IN CHAR16   *Buffer,
    IN BOOLEAN  Comma,
    IN INT64    v
) {
    STATIC CHAR8 ca[] = { 3, 1, 2 };
    CHAR8        str[40], *p1;
    CHAR16       *p2;
    UINTN        c, r;

    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }

    p1 = str;
    p2 = Buffer;

    if (v < 0) {
        *(p2++) = '-';
        v = -v;
    }

    while (v) {
        r = v % 10;
        v = v / 10;
        *(p1++) = (CHAR8)r + '0';
    }

    c = (Comma ? ca[(p1 - str) % 3] : 999) + 1;
    while (p1 != str) {

        c -= 1;
        if (!c) {
            *(p2++) = ',';
            c = 3;
        }

        *(p2++) = *(--p1);
    }
    *p2 = 0;
}

VOID FloatToString (
    IN CHAR16   *Buffer,
    IN BOOLEAN  Comma,
    IN double   v
) {
    /*
     * Integer part.
     */
    INTN i = (INTN)v;
    ValueToString(Buffer, Comma, i);


    /*
     * Decimal point.
     */
    UINTN x = StrLen(Buffer);
    Buffer[x] = L'.';
    x++;


    /*
     * Keep fractional part.
     */
    float f = (float)(v - i);
    if (f < 0) f = -f;


    /*
     * Leading fractional zeroes.
     */
    f *= 10.0;
    while (   (f != 0)
           && ((INTN)f == 0))
    {
      Buffer[x] = L'0';
      x++;
      f *= 10.0;
    }


    /*
     * Fractional digits.
     */
    while ((float)(INTN)f != f)
    {
      f *= 10;
    }
    ValueToString(Buffer + x, FALSE, (INTN)f);
    return;
}

VOID TimeToString (
    OUT CHAR16      *Buffer,
    IN EFI_TIME     *Time
) {
    UINTN       Hour, Year;
    CHAR16      AmPm;

    AmPm = 'a';
    Hour = Time->Hour;
    if (Time->Hour == 0) {
        Hour = 12;
    } else if (Time->Hour >= 12) {
        AmPm = 'p';
        if (Time->Hour >= 13) {
            Hour -= 12;
        }
    }

    Year = Time->Year % 100;

    // bugbug: for now just print it any old way
    SPrint (Buffer, 0, L"%02d/%02d/%02d  %02d:%02d%c",
        Time->Month,
        Time->Day,
        Year,
        Hour,
        Time->Minute,
        AmPm
        );
}
