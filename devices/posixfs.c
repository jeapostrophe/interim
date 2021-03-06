#include <stdio.h>
#include "minilisp.h"
#include "alloc.h"
#include "stream.h"
#include "compiler_new.h"
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

Cell* _file_cell;

Cell* posixfs_open(Cell* cpath) {
  char* path;
  _file_cell = alloc_nil();

  if (!cpath || cpath->tag!=TAG_STR) {
    printf("[posixfs] open error: non-string path given\r\n");
    return _file_cell;
  }

  path = cpath->ar.addr;
  
  if (!strncmp(path,"/sd/",4)) {
    char* name = NULL;
    char* filename = NULL;

    if (strlen(path)>4) {
      filename = path+4;
    }

    if (!filename || !filename[0]) filename = ".";

    printf("filename: %s\r\n",filename);
    
    if (filename) {
      DIR* dirp;
      FILE* f;
      if ((dirp = opendir(filename))) {
        struct dirent *dp;
        Cell* nl = alloc_string_copy("\n");
        _file_cell = alloc_string_copy("");
        
        do {
          if ((dp = readdir(dirp)) != NULL) {
            _file_cell = alloc_concat(_file_cell,alloc_concat(alloc_string_copy(dp->d_name),nl));
          }
        } while (dp != NULL);
        return _file_cell;
      }

      f = fopen(filename, "rb");
      if (f) {
        Cell* res;
        int len, read_len;
        fseek(f, 0L, SEEK_END);
        len = ftell(f);
        fseek(f, 0L, SEEK_SET);
        
        printf("[posixfs] trying to read file of len %d…\r\n",len);
        res = alloc_num_bytes(len);
        read_len = fread(res->ar.addr, 1, len, f);
        // TODO: close?
        _file_cell = res;
        return res;
      } else {
        // TODO should return error
        printf("[posixfs] could not open file :(\r\n");
        _file_cell = alloc_string_copy("<error: couldn't open file.>"); // FIXME hack
        return _file_cell;
      }
      _file_cell = alloc_string_copy("<error: file not found.>");
      return _file_cell;
    } else {
      // TODO dir
    }
  }

  return _file_cell;
}

Cell* posixfs_read(Cell* stream) {
  return _file_cell;
}

Cell* posixfs_write(Cell* arg) {
  return NULL;
}

Cell* posixfs_mmap(Cell* arg) {
  return alloc_nil();
}

void mount_posixfs() {
  fs_mount_builtin("/sd", posixfs_open, posixfs_read, posixfs_write, 0, posixfs_mmap);
}

