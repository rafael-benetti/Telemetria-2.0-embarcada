#include "includes.h"
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        DBG_PRINTLN("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        DBG_PRINTLN("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            DBG_PRINTLN("  DIR : ");
            DBG_PRINTLN(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            DBG_PRINTLN("  FILE: ");
            DBG_PRINTLN(file.name());
            DBG_PRINTLN("  SIZE: ");
            DBG_PRINTLN(file.size());
        }
        file = root.openNextFile();
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        DBG_PRINTLN("Failed to open file for reading");
        return;
    }

    DBG_PRINTLN("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        DBG_PRINTLN("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        DBG_PRINTLN("File written");
    } else {
        DBG_PRINTLN("Write failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        DBG_PRINTLN("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        DBG_PRINTLN("Message appended");
    } else {
        DBG_PRINTLN("Append failed");
    }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        DBG_PRINTLN("File renamed");
    } else {
        DBG_PRINTLN("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        DBG_PRINTLN("File deleted");
    } else {
        DBG_PRINTLN("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        DBG_PRINTLN("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        DBG_PRINTLN("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}