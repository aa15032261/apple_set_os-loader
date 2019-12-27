const path = require('path');
const fs = require('fs');
const readline = require('readline');

const readInterface = readline.createInterface({
    input: fs.createReadStream(path.resolve(__dirname, './pci.ids.txt'))
});


let vendors = {

};

let current_vendor = NaN;
let current_vendor_name = null;

let graphicsBrands = {
    'nvidia': ['titan', 'geforce', 'quadro'],
    'amd': ['radeon', 'firepro', 'vega'],
    'intel': ['hd graphics', 'iris']
}


readInterface.on('line', function(line) {
    if (line.startsWith('#')) {
        return;
    }
    if (line.startsWith('C')) {
        current_vendor = NaN;
        return;
    }
    if (line.length < 4) {
        return;
    }


    if (!isNaN(current_vendor) && line.startsWith('\t')) {
        let device_name = line.substr(7);

        let device_name_lower = device_name.toLowerCase();

        for (let name of graphicsBrands[current_vendor_name]) {
            if (device_name_lower.indexOf(name) !== -1) {
                vendors[current_vendor].devices.push({
                    device_id: parseInt(line.substr(1, 4), 16),
                    device_name: line.substr(7)
                });
                break;
            }
        }
        return;
    }

    let vendor_name = line.substr(6);

    if (vendor_name.toLowerCase().indexOf('intel corp') === 0) {
        current_vendor_name = 'intel';
    } else if (vendor_name.toLowerCase().indexOf('nvidia') === 0) {
        current_vendor_name = 'nvidia';
    } else if (vendor_name.toLowerCase().indexOf('advanced micro devices') === 0) {
        current_vendor_name = 'amd';
    } else {
        current_vendor_name = null;
        current_vendor = NaN;
    }

    if (current_vendor_name) {
        current_vendor = parseInt(line.substr(0, 4), 16);
        vendors[current_vendor] = {
            vendor_id: current_vendor,
            vendor_name: current_vendor_name.toUpperCase(),
            devices: []
        };
    }

}).on('close', function () {
    let vendor_count = 0;

    let pci_db_c = '';



    let pci_vendor_db = '';

    let pci_vendor_sections = [];

    for (let vendor_id in vendors) {
        if (vendors[vendor_id].devices.length > 0) {
            vendor_count++;
            pci_vendor_db += ', &pci_' + vendor_id;
    
            let currentSection = 'PCI_VENDOR_DB pci_' + vendor_id + ' = {\r\n';
            currentSection += '    ' + vendor_id + ', \r\n';
            currentSection += '    L"' + vendors[vendor_id].vendor_name.replace(/"/g, '\\"').substring(0, 12) + '",\r\n';
            currentSection += '    ' + vendors[vendor_id].devices.length + ',\r\n';
            currentSection += '    ' + '{ \r\n';
    
            for (let device of vendors[vendor_id].devices) {
                currentSection += '        ' + '{' + device.device_id + ', ' + 'L"';
                let device_name_filtered = device.device_name.replace(/"/g, '\\"');

                if (device_name_filtered.length > 63) {
                    device_name_filtered = device_name_filtered.substring(0, 60) + '...';
                }

                currentSection += device_name_filtered + '"},\r\n';
            }
            currentSection = currentSection.substring(0, currentSection.length - 3);
            
            currentSection += '\r\n    }' + '\r\n';
            currentSection += '};\r\n\r\n'
    
            pci_vendor_sections.push(currentSection);
        }
    }

    pci_vendor_db = pci_vendor_db.substring(2);

    pci_db_c += '#include "../include/pci_db.h"\r\n\r\n';

    for (let section of pci_vendor_sections) {
        pci_db_c += section;
    }

    pci_db_c += '\r\n';
    pci_db_c += 'PCI_VENDOR_DB*  _pci_vendor_db[' + vendor_count + ']    = {' + pci_vendor_db + '};\r\n';
    pci_db_c += 'PCI_VENDOR_DB** pci_vendor_db        = _pci_vendor_db;\r\n';
    pci_db_c += 'UINT16          pci_vendor_db_size   = ' + vendor_count + ';\r\n\r\n';

    fs.writeFileSync(path.resolve(__dirname, '../lib/pci_db.c'), pci_db_c, {encoding:'utf8',flag:'w'});
});




