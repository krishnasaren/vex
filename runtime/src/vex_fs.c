/* vex_fs.c — File system operations */
#include "../include/vex_rt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

VexFile vex_file_open(VexStr path, uint32_t flags) {
    char buf[4096];
    size_t n = path.len < 4095 ? path.len : 4095;
    memcpy(buf, path.ptr, n); buf[n] = '\0';
    const char* mode = (flags==0)?"rb":(flags==1)?"wb":(flags==2)?"r+b":"ab";
    return (VexFile)fopen(buf, mode);
}
bool    vex_file_valid(VexFile f) { return f != NULL; }
int64_t vex_file_read(VexFile f, uint8_t* buf, size_t len) {
    return (int64_t)fread(buf,1,len,(FILE*)f);
}
int64_t vex_file_write(VexFile f, const uint8_t* buf, size_t len) {
    return (int64_t)fwrite(buf,1,len,(FILE*)f);
}
void    vex_file_close(VexFile f) { if(f) fclose((FILE*)f); }
bool    vex_file_exists(VexStr path) {
    char buf[4096];
    size_t n = path.len<4095?path.len:4095;
    memcpy(buf,path.ptr,n); buf[n]='\0';
    return access(buf, F_OK) == 0;
}
bool    vex_file_delete(VexStr path) {
    char buf[4096];
    size_t n=path.len<4095?path.len:4095;
    memcpy(buf,path.ptr,n); buf[n]='\0';
    return remove(buf) == 0;
}
