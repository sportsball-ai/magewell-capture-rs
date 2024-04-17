#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "mw_mp4/mw_mp4.h"
#include <dirent.h>
void print_useage()
{
    printf("Mp4Repair [filename]/[dir]\n");
}

int repair_file(char *p_mp4_file, char *p_info_file)
{
    mw_mp4_status_t status = mw_mp4_repair(p_mp4_file, true);
    if(status != MW_MP4_STATUS_SUCCESS){
        printf("repair %s fail\n",p_mp4_file);
        return 1;
    }
    printf("repair %s success\n",p_mp4_file);
}

int repair_file_search(char *p_path, char *p_file_name)
{
    char mp4_file[256];
    char info_file[256];
    int len;
    sprintf(info_file, "%s/%s", p_path, p_file_name);
    len = strlen(info_file);
    if (len <= 260) {
        memcpy(mp4_file, info_file, len - 5);
        mp4_file[len - 5] = 0;
        repair_file(mp4_file, info_file);
    }
}

int repair_dir(char *p_path)
{
    DIR *dir=NULL;
    struct dirent *entry; 
    if((dir = opendir(p_path))==NULL){
         printf("opendir failed!");
         return 1;
    }
    while(entry=readdir(dir)){
        int len = strlen(entry->d_name);
        if((len > 9) && (!memcmp(entry->d_name + len-9, ".mp4.info", 9))){
            repair_file_search(p_path, entry->d_name);
        }
    }        
    closedir(dir);
    return 0;
}
int main(int argc, char* argv[])
{
    int str_len;
    char info_file[256];
    if(argc < 2){
        print_useage();
        return -1;
    }
    str_len = strlen(argv[1]);
    if(0 == memcmp(argv[1] + str_len-4, ".mp4",4)){
        sprintf(info_file, "%s.info", argv[1]);
        return repair_file(argv[1], info_file);
    }
    return repair_dir(argv[1]);
}
