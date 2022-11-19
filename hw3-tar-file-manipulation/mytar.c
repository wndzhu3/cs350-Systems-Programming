// Wendy Zhu
// wzhu62@emory.edu
/* THIS  CODE  WAS MY OWN WORK , IT WAS  WRITTEN  WITHOUT  CONSULTING  ANY
SOURCES  OUTSIDE  OF  THOSE  APPROVED  BY THE  INSTRUCTOR. Wendy Zhu  */

#include "inodemap.c"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

void createHelper(FILE *archive, char *directory) {
  struct dirent *d;
  struct stat file_stats;
  char *name;

  DIR *input_dir = opendir(directory);
  name = malloc(strlen(directory) + 258);

  if (input_dir == NULL) {
    perror("opendir");
    exit(-1);
  }
  if (name == NULL) {
    perror("malloc");
    exit(-1);
  }

  d = readdir(input_dir);
  while (d != NULL) {
    if (!sprintf(name, "%s/%s", directory, d->d_name)) {
      perror("sprintf");
      exit(-1);
    } else if (lstat(name, &file_stats) != 0) {
      perror("lstat");
      exit(-1);
    } else if (!S_ISLNK(file_stats.st_mode) && strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0) {
      int64_t i = file_stats.st_ino;
      int32_t l = strlen(name);

      if (!fwrite(&i, 8, 1, archive) || !fwrite(&l, 4, 1, archive) || !fwrite(name, l, 1, archive)) {
        perror("fwrite");
        exit(-1);
      } else if (!get_inode(file_stats.st_ino)) {
        set_inode(file_stats.st_ino, name);
        int32_t m = file_stats.st_mode;
        int64_t t = (int64_t)file_stats.st_mtime;

        if (!fwrite(&m, 4, 1, archive) || !fwrite(&t, 8, 1, archive)) {
          perror("fwrite");
          exit(-1);
        } else if (!S_ISDIR(file_stats.st_mode)) {
          int64_t size = file_stats.st_size;
          if (!fwrite(&size, 8, 1, archive)) {
            perror("fwrite");
            exit(-1);
          } else if (size > 0) {
            FILE *input = fopen(name, "r");
            char *contents = malloc(sizeof(char) * file_stats.st_size);

            if (input == NULL) {
              perror("fopen");
              exit(-1);
            } else if (!fwrite(contents, file_stats.st_size, 1, archive)) {
              perror("fwrite");
              exit(-1);
            } else if (!fread(contents, file_stats.st_size, 1, input)) {
              perror("fread");
              exit(-1);
            }

            int f = fclose(input);
            if (f != 0) {
              perror("fclose");
              exit(-1);
            }
          }
        }
      }
    }

    if (S_ISDIR(file_stats.st_mode)) {
      if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0) {
        createHelper(archive, name);
      }
    }
    d = readdir(input_dir);
  }
  if (closedir(input_dir) != 0) {
    perror("closedir");
    exit(-1);
  }
}

void createArchive(char *file, char *directory) {
  FILE *archive = fopen(file, "w+");

  if (archive == NULL) {
    perror("fopen");
    exit(-1);
  }

  int32_t magic = 0x7261746D;
  struct stat stats;
  int64_t inode = stats.st_ino;

  if (!fwrite(&magic, 4, 1, archive) || !fwrite(&inode, 8, 1, archive)) {
    perror("fwrite");
    exit(-1);
  }
  if (lstat(directory, &stats) != 0) {
    perror("lstat");
    exit(-1);
  }

  set_inode(stats.st_ino, directory);

  int32_t name_length = strlen(directory);
  int32_t mode = stats.st_mode;
  int64_t time = stats.st_mtime;

  if (!fwrite(&name_length, 4, 1, archive) || !fwrite(directory, name_length, 1, archive)
      || !fwrite(&mode, 4, 1, archive) || !fwrite(&time, 8, 1, archive)) {
    perror("fwrite");
    exit(-1);
  }

  createHelper(archive, directory);

}

void printContents(char *file) {
  FILE *archive = fopen(file, "r");
  int32_t magic;

  if (archive == NULL) {
    perror("fopen");
    exit(-1);
  } else if (!fread(&magic, 4, 1, archive)) {
    perror("fread");
    exit(-1);
  } else if (magic != 0x7261746D) {
    fprintf(stderr, "Error: Bad magic number (%d), should be: %d.\n", magic, 0x7261746D);
    exit(-1);
  }

  int64_t inode_int;
  ino_t inode = 0;

  while (fread(&inode_int, 8, 1, archive) == 1) {
    inode = (ino_t) inode_int;
    int32_t name_length;
    char *filename = malloc(sizeof(char) * name_length);

    if (!fread(&name_length, 4, 1, archive)) {
      perror("fread");
      exit(-1);
    } else if (filename == NULL) {
      perror("malloc");
      exit(-1);
    } else if (!fread(filename, name_length, 1, archive)) {
      perror("fread");
      exit(-1);
    }

    filename[name_length] = '\0';
    if (get_inode(inode) == NULL) {
      mode_t rm;
      int32_t rm_int;
      rm = (mode_t) rm_int;

      int64_t read_mtime_int;

      if (fread(&rm_int, 4, 1, archive) == -1) {
        perror("fread");
        exit(-1);
      } else if (!fread(&read_mtime_int, 8, 1, archive)) {
        perror("fread");
        exit(-1);
      }

      set_inode(inode, filename);
      time_t read_mtime = read_mtime_int;

      if (S_ISDIR(rm)) {
        printf("%s/ inode: %llu, mode: %o, mtime: %llu\n", filename, inode, rm, (unsigned long long) read_mtime);
      } else {
        int64_t read_size_int;

        if (!fread(&read_size_int, 8, 1, archive)) {
          perror("fread");
          exit(-1);
        }

        off_t read_size = (off_t) read_size_int;
        if (read_size == 0) {
          if ((rm & S_IXGRP) || (rm & S_IXOTH) || (rm & S_IXUSR)) {
            printf("%s* inode: %llu, mode: %o, mtime: %llu, size: %llu\n", filename, inode, rm, (unsigned long long) read_mtime, read_size);
            break;
          }
          printf("%s inode: %llu, mode: %o, mtime: %llu, size: %llu\n", filename, inode, rm, (unsigned long long) read_mtime, read_size);
        } else {
          char *contents = malloc(sizeof(char) * read_size);
          if (contents == NULL) {
            perror("malloc");
            exit(-1);
          } else if (!fread(contents, read_size, 1, archive)) {
            perror("fread");
            exit(-1);
          }
        }
      }
    } else {
      printf("%s/ inode: %llu\n", filename, inode);
    }
  }
  if (fclose(archive) != 0) {
    perror("fclose");
    exit(-1);
  }
}

void extractArchiveHelper(FILE *archive) {
  int64_t inode_int;

  while (fread(&inode_int, 8, 1, archive) == 1) {
    ino_t inode = inode_int;
    int32_t name_length;

    if (!fread(&name_length, 4, 1, archive)) {
      perror("fread");
      exit(-1);
    }

    char *filename =  malloc(sizeof(char) * (name_length + 1));
    if (filename == NULL) {
      perror("malloc");
      exit(-1);
    } else if (!fread(filename, name_length, 1, archive)){
      perror("fread");
      exit(-1);
    }

    filename[name_length] = '\0';

    if (get_inode(inode) == NULL) {
      int32_t mode;
      int64_t file_mtime;

      if (!fread(&mode, 4, 1, archive) || !fread(&file_mtime, 8, 1, archive)) {
        perror("fread");
        exit(-1);
      } else if (S_ISDIR((mode_t)mode)) {
        if (mkdir(filename, (mode_t)mode) != 0) {
          perror("mkdir");
          exit(-1);
        }
        set_inode(inode, filename);
      } else {
        int64_t size;
        FILE *curr = fopen(filename, "w+");

        if (!fread(&size, 8, 1, archive)){
          perror("fread");
          exit(-1);
        }
        if (curr == NULL) {
          perror("fopen");
          exit(-1);
        }
        if (size != 0) {
          char *contents = malloc(sizeof(char) * size);
          if (contents == NULL) {
            perror("malloc");
            exit(-1);
          } else if (!fread(contents, size, 1, archive)) {
            perror("fread");
            exit(-1);
          } else if (!fwrite(contents, size, 1, curr)) {
            perror("fwrite");
            exit(-1);
          }
          free(contents);
        }

        struct timeval time_ac;
        struct timeval time_mod;

        if (chmod(filename, (mode_t)mode) != 0){
          perror("fread");
          exit(-1);
        } else if (gettimeofday(&time_ac, NULL) != 0){
          perror("gettimeofday");
          exit(-1);
        }

        time_mod.tv_usec = 0;
        time_mod.tv_sec = (time_t) file_mtime;

        struct timeval t[2] = {time_ac, time_mod};
        set_inode(inode, filename);

        if (utimes(filename, t) != 0){
          perror("utimes");
          exit(-1);
        }

        int c = fclose(curr);

        if (c != 0){
          perror("fclose");
          exit(-1);
        }
      }
    } else {
      char *src = malloc(sizeof(char) * (1 + strlen(get_inode(inode))));
      if (src == NULL){
        perror("malloc");
        exit(-1);
      } else if (strcpy(src, get_inode(inode)) == NULL){
        perror("strcpy");
        exit(-1);
      } else if (link(src, filename) != 0){
        perror("link");
        exit(-1);
      }
      free(src);
    }
  }
}

void extractArchive(char *file) {
  FILE *archive = fopen(file, "r");
  int32_t magic;

  if (archive == NULL) {
    perror("fopen");
    exit(-1);
  } else if (!fread(&magic, 4, 1, archive)) {
    perror("fread");
    exit(-1);
  } else if (magic != 0x7261746D) {
    fprintf(stderr, "Error: Bad magic number (%d), should be: %d.\n", magic, 0x7261746D);
    exit(-1);
  }

  extractArchiveHelper(archive);

  int f = fclose(archive);
  if (f != 0) {
    perror("fclose");
    exit(-1);
  }
}

int main(int argc, char *argv[]) {
  char option;
  char * tarfile;
  char * directory;

  int opt;
  int n = 0;

  while ((opt = getopt(argc, argv, "cxtf:")) != -1) {
    switch (opt) {
      case 'c':
        n++;
        if (n > 1) {
          fprintf(stderr, "Error: Multiple modes specified.\n");
          exit(-1);
        }
        option = 'c';
        break;
      case 'x':
        n++;
        if (n > 1) {
          fprintf(stderr, "Error: Multiple modes specified.\n");
          exit(-1);
        }
        option = 'x';
        break;
      case 't':
        n++;
        if (n > 1) {
          fprintf(stderr, "Error: Multiple modes specified.\n");
          exit(-1);
        }
        option = 't';
        break;
      case 'f':
        tarfile = optarg;
        break;
      default:
        fprintf(stderr, "Error: No tarfile specified.\n");
        exit(-1);
    }
  }

  switch (option) {
    case 'c':
      while (optind < argc) {
        directory = argv[optind];
        optind++;
      }
      if (directory == NULL) {
        fprintf(stderr, "Error: No directory target specified.\n");
        exit(-1);
      } else {
        struct stat stats;
        if (lstat(directory, &stats) != 0) {
          fprintf(stderr, "Error: Specified target (\"%s\") does not exist.\n", directory);
          exit(-1);
        } else if (!S_ISDIR(stats.st_mode)) {
          fprintf(stderr, "Error: Specified target (\"%s\") is not a directory.\n", directory);
          exit(-1);
        }
      }
      createArchive(tarfile, directory);
      break;
    case 'x':
      extractArchive(tarfile);
      break;
    case 't':
      printContents(tarfile);
      break;
    default:
      fprintf(stderr, "Error: No mode specified.\n");
      exit(-1);
  }

  exit(0);
}
