#include <midi/RolandSysex.h>
#include <Logger.h>

using namespace MIDI;
using namespace LOGGING;

void RolandSysex::checksumAdd(int& checksum, uint8_t val) {
    checksum += val;
    if (checksum > 127) {
        checksum -= 128;
    }
}

uint8_t RolandSysex::checksum(RolandAddr& addr, uint8_t bytes[], uint16_t len) {
    int res = 0;

    checksumAdd(res, addr.hsb);    
    checksumAdd(res, addr.msb);    
    checksumAdd(res, addr.lsb);    

    for (int i = 0; i < len; i++) {
        checksumAdd(res, bytes[i]);    
    }

    res = 128 - res;
    return res == 128 ? 0 : res;
}

int RolandSysex::readSysEx(MidiReader* r, MidiWriter* w, RolandSysexCallback* cb) {

    RolandSysexHdr hdr;
    RecordInfo recordInfo;
    int recordCount;

    uint8_t* buffer = 0;
    int checksum = 0, checkbyte = 0;
    size_t len, bufsize;

    Logger::defaultLogger().debug("Roland:Sysex - Read Header.");
    len = r->readSysEx(__buffer(&hdr), ROLAND_SYSEX_HDR_SIZE);

    if (len < ROLAND_SYSEX_HDR_SIZE) {
        Logger::defaultLogger().warn("Roland:Sysex - Header could not be read (received %d bytes). Stop.", len);
        Logger::defaultLogger().dump("Roland header received", __buffer(&hdr), len, 0);
        return -1;
    }

    Logger::defaultLogger().debug("Roland:Sysex - Check device-ID.");
    if (hdr.device != cb->getDeviceID()) {
        Logger::defaultLogger().warn("Roland:Sysex - Device-ID does not match. Expected: %x, Received: %x", cb->getDeviceID(), hdr.device);
        return -1;
    }

    Logger::defaultLogger().debug("Roland:Sysex - Check model-ID.");
    if (hdr.model != cb->getModelID()) {
        Logger::defaultLogger().warn("Roland:Sysex - Model-ID does not match. Expected: %x, Received: %x", cb->getModelID(), hdr.model);
        return -1;
    }

    if (hdr.cmd == ROLAND_SYSEX_CMD_READ) {
        Logger::defaultLogger().debug("Roland:Sysex:Read - Parse pull request.");

        buffer = new uint8_t[4]; // for reading the size to be sent outside.
        if (r->readSysEx(buffer, 4) < 4) {
            Logger::defaultLogger().warn("Roland:Sysex:Read - Did not receive next 4 bytes. Stop here.");
            delete buffer;
            return -1;
        }        

        recordCount = cb->getRecordCount(hdr.addr);
        if (recordCount == -1) {
            Logger::defaultLogger().warn("Roland:Sysex:Read - Requested address is invalid: %x:%x:%x", hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb);
            delete buffer;
            return -1;
        }

        // check checksum
        checksum = RolandSysex::checksum(hdr.addr, buffer, 3);    // calculate checksum from hdr + length (3 bytes from buffer)        
        delete buffer; // delete buffer, as this one is no longer needed.
        if (checksum != buffer[3]) {
            Logger::defaultLogger().warn("Roland:Sysex:Read - Calculated checksum does not match received: %x, calculated: %x", buffer[3], checksum);            
            return -1;
        }

        // get record size
        len = (__lsb(buffer[0]) << 14) | (__lsb(buffer[1]) << 7) | __lsb(buffer[2]);
        Logger::defaultLogger().debug("Roland:Sysex:Read - Size requested %d.", len);

        hdr.cmd = ROLAND_SYSEX_CMD_WRITE; // Set the response command to SET

        for (int rec = 0; rec < recordCount; rec++) {

            if (!cb->getRecordInfo(hdr.addr, rec, recordInfo)) {
                Logger::defaultLogger().warn("Invalid address / record num: %x:%x:%x > %x", hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb, rec);
                continue;
            }

            if (len != recordInfo.size) {
                Logger::defaultLogger().warn("Size requested %d does not match record size %d at %x:%x:%x > %x", len, recordInfo.size, hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb, rec);
                continue;
            }

            bufsize = ROLAND_SYSEX_HDR_SIZE + recordInfo.size + 1;
            buffer = new uint8_t[bufsize];

            // copy header without address to buffer
            memcpy(buffer, &hdr, ROLAND_SYSEX_HDR_SIZE - 3);    

            // copy address to buffer
            memcpy(buffer + (ROLAND_SYSEX_HDR_SIZE - 3), &(recordInfo.addr), 3);    

            // copy data to buffer
            cb->read(recordInfo.addr, (buffer + ROLAND_SYSEX_HDR_SIZE), recordInfo.size); 

            // create checksum and add to buffer
            checksum = RolandSysex::checksum(recordInfo.addr, (buffer + ROLAND_SYSEX_HDR_SIZE), recordInfo.size);
            memcpy((buffer + ROLAND_SYSEX_HDR_SIZE + recordInfo.size), &checksum, 1);

            Logger::defaultLogger().debug("Roland:Sysex:Read - Send sysex checksum:%x len:%d.", checksum, bufsize);
            w->sendSysEx(ROLAND_SYSEX_MAN_CODE, buffer, bufsize);

            delete buffer;
        }

        return len;
    }
    else if (hdr.cmd == ROLAND_SYSEX_CMD_WRITE) {
        Logger::defaultLogger().debug("Roland:Sysex - Parse set request.");

        recordCount = cb->getRecordCount(hdr.addr);
        if (recordCount < 1) {
            Logger::defaultLogger().warn("Invalid address for write command: %x:%x:%x. Stop.", hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb);
            return -1;
        }

        if (!cb->getRecordInfo(hdr.addr, 0, recordInfo)) {
            Logger::defaultLogger().warn("Record Info was not provided for address: %x:%x:%x. Stop.", hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb);
            return -1;
        }

        bufsize = recordInfo.size + 1;      // add checksum byte
        buffer = new uint8_t[bufsize];      // buffer is size + 1 byte for checksum

        len = r->readSysEx(buffer, bufsize);       // read buffer incl. checksum
        if (len < bufsize) {
            Logger::defaultLogger().warn("Could not read the sysex buffer of size %d. Received %d instead. Stop.", bufsize, len);
            delete buffer;
            return -1;
        }

        checksum = RolandSysex::checksum(hdr.addr, buffer, recordInfo.size);
        if (checksum != buffer[recordInfo.size]) {
            Logger::defaultLogger().warn("Roland:Sysex:Write - Calculated checksum does not match received: %x, calculated: %x", buffer[recordInfo.size], checksum);
            delete buffer;
            return -1;
        }

        cb->write(hdr.addr, buffer, recordInfo.size);
        delete buffer;

        return recordInfo.size;
    }
    else {
        Logger::defaultLogger().warn("Roland:Sysex - invalid command received: %x", hdr.cmd);
        return -1;
    }


}