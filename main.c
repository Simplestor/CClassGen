#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <ctype.h>

#define MAX_ROW_LENGTH  4096
#define MAX_FILE_NAME   128
#define GENERATE_PATH   "./gen"

int strupr(char* dst,char* src)
{
    if(!src || !dst){
        return -EINVAL;
    }
    while (*src != '\0'){
        if(*src >= 'a' && *src <= 'z'){
            *dst = (*src) - 0x20;
        }else{
            *dst = *src;
        }
        src++;
        dst++;
    }
    *dst = '\0';
    return 0;
}

int generate_h(char* name)
{
    char filename[MAX_FILE_NAME];
    char uname[MAX_FILE_NAME];
    char row[MAX_ROW_LENGTH];
    FILE* fp;
    
    sprintf(filename,"%s/%s.h",GENERATE_PATH,name);
    if(!access(filename,R_OK)){
        printf("%s have existed, will be replaced\n",filename);
        remove(filename);
    }
    fp = fopen(filename,"w+");
    if(!fp){
        printf("open h file error\n");
        return -EFAULT;
    }
    strupr(uname,name);
    sprintf(row,"%s#ifndef __%s_H__ \n","",uname);
    sprintf(row,"%s#define __%s_H__\n",row,uname);
    sprintf(row,"%s#include <stdint.h>\n",row);
    sprintf(row,"%s#include \"lock.h\"\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"struct %s;\n\n",name);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"%senum %s_EVENT\n","",uname);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    %s_EVENT_NONE = 0,\n",row,uname);
    sprintf(row,"%s    %s_EVENT_MAX,\n",row,uname);
    sprintf(row,"%s};\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"typedef int32_t (*%s_event_notify)(struct %s*, int32_t, void*, void*);\n\n",name,name);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"%sstruct %s_event_action\n","",name);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    %s_event_notify notify;\n",row,name);
    sprintf(row,"%s    void* object;\n",row);
    sprintf(row,"%s    struct %s_event_action* next;\n",row,name);
    sprintf(row,"%s};\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"%sstruct %s_operation\n{\n","",name);
    sprintf(row,"%s    int32_t (*init)(struct %s*);\n",row,name);
    sprintf(row,"%s    int32_t (*release)(struct %s*);\n",row,name);
    sprintf(row,"%s    int32_t (*register_notify)(struct %s*, int32_t, %s_event_notify notify, void*);\n",row,name,name);
    sprintf(row,"%s    int32_t (*unregister_notify)(struct %s*, int32_t, void*);\n",row,name);
    sprintf(row,"%s    int32_t (*trigger_notify)(struct %s*, int32_t, void*);\n",row,name);
    sprintf(row,"%s};\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"%sstruct %s\n{\n","",name);
    sprintf(row,"%s    lock_t lock;\n",row,name);
    sprintf(row,"%s    struct %s_operation* op;\n",row,name);
    sprintf(row,"%s    struct %s_event_action *paction[%s_EVENT_MAX];\n",row,name,uname);
    
    sprintf(row,"%s};\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"%sint32_t create_init_%s(struct %s** p%s);\n","",name,name,name);
    sprintf(row,"%svoid release_destroy_%s(struct %s* p%s);\n",row,name,name,name);
    sprintf(row,"%s\n#endif\n",row);
    fwrite(row,strlen(row),1,fp);

    fclose(fp);
    return 0;
}

int generate_c(char* name)
{
    char filename[MAX_FILE_NAME];
    char row[MAX_ROW_LENGTH];
    char uname[MAX_FILE_NAME];
    FILE* fp;
    sprintf(filename,"%s/%s.c",GENERATE_PATH,name);
    if(!access(filename,R_OK)){
        printf("%s have existed, will be replaced\n",filename);
        remove(filename);
    }
    fp = fopen(filename,"w+");
    if(!fp){
        printf("open c file error\n");
        return -EFAULT;
    }
    strupr(uname,name);

    sprintf(row,"%s#include <stdlib.h>\n","");
    sprintf(row,"%s#include <string.h>\n",row);
    sprintf(row,"%s#include <errno.h>\n",row);
    sprintf(row,"%s#include \"log.h\"\n",row);
    sprintf(row,"%s#include \"%s.h\"\n\n",row,name);
    fwrite(row,strlen(row),1,fp);
    
    sprintf(row,"%sstatic int32_t %s_init(struct %s* p%s)\n","",name,name,name);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    lock_init(&(p%s->lock));\n",row,name);
    sprintf(row,"%s    return 0;\n",row);
    sprintf(row,"%s}\n\n",row);
    fwrite(row,strlen(row),1,fp);
    
    sprintf(row,"%sstatic int32_t %s_release(struct %s* p%s)\n","",name,name,name);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    lock_destroy((&p%s->lock));\n",row,name);
    sprintf(row,"%s    return 0;\n",row);
    sprintf(row,"%s}\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"%sstatic int32_t %s_register_notify(struct %s* p%s, int32_t event, %s_event_notify notify, void* object)\n","",name,name,name,name);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    struct %s_event_action* paction;\n",row,name);
    sprintf(row,"%s    if(!notify || (event <= %s_EVENT_NONE) || (event >= %s_EVENT_MAX)){\n",row,uname,uname);
    sprintf(row,"%s        return -EINVAL;\n",row);
    sprintf(row,"%s    }\n",row);

    sprintf(row,"%s    paction = (struct %s_event_action*)malloc(sizeof(struct %s_event_action));\n",row,name,name);
    sprintf(row,"%s    if(!paction){\n",row);
    sprintf(row,"%s        DBG(DBG_ERR,\"malloc error\\n\");\n",row);
    sprintf(row,"%s        return -ENOMEM;\n",row);
    sprintf(row,"%s    }\n",row);
    
    sprintf(row,"%s    paction->notify = notify;\n",row);
    sprintf(row,"%s    paction->object = object;\n",row);
    sprintf(row,"%s    lock(&(p%s->lock));\n",row,name);
    sprintf(row,"%s    paction->next = p%s->paction[event];\n",row,name);
    sprintf(row,"%s    p%s->paction[event] = paction;\n",row,name);
    sprintf(row,"%s    unlock(&(p%s->lock));\n",row,name);

    sprintf(row,"%s    return 0;\n",row);
    sprintf(row,"%s}\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"%sstatic int32_t %s_unregister_notify(struct %s* p%s, int32_t event, void* object)\n","",name,name,name);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    struct %s_event_action *paction,* ptmp;\n",row,name);
    sprintf(row,"%s    if((event <= %s_EVENT_NONE) || (event >= %s_EVENT_MAX)){\n",row,uname,uname);
    sprintf(row,"%s        return -EINVAL;\n",row);
    sprintf(row,"%s    }\n",row);
    sprintf(row,"%s    lock(&(p%s->lock));\n",row,name);
    sprintf(row,"%s    paction = p%s->paction[event];\n",row,name);
    sprintf(row,"%s    if(paction->object == object){\n",row,name);
    sprintf(row,"%s        p%s->paction[event] = paction->next;\n",row,name);
    sprintf(row,"%s        free(paction);\n",row);
    sprintf(row,"%s    }else{\n",row);
    sprintf(row,"%s        while(paction->next){\n",row);
    sprintf(row,"%s            if(paction->next->object == object){\n",row);
    sprintf(row,"%s                ptmp = paction->next;\n",row);
    sprintf(row,"%s                paction->next = ptmp->next;\n",row);
    sprintf(row,"%s                free(ptmp);\n",row);
    sprintf(row,"%s                break;\n",row);
    sprintf(row,"%s            }\n",row);
    sprintf(row,"%s            paction = paction->next;\n",row);
    sprintf(row,"%s        }\n",row);
    sprintf(row,"%s    }\n",row);
    sprintf(row,"%s    unlock(&(p%s->lock));\n",row,name);
    sprintf(row,"%s    return 0;\n",row);
    sprintf(row,"%s}\n\n",row);
    fwrite(row,strlen(row),1,fp);
    
    sprintf(row,"%sstatic int32_t %s_trigger_notify(struct %s* p%s, int32_t event, void* context)\n","",name,name,name);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    struct %s_event_action* paction;\n",row,name);
    sprintf(row,"%s    if((event <= %s_EVENT_NONE) || (event >= %s_EVENT_MAX)){\n",row,uname,uname);
    sprintf(row,"%s        return -EINVAL;\n",row);
    sprintf(row,"%s    }\n",row);
    sprintf(row,"%s    paction = p%s->paction[event];\n",row,name);
    sprintf(row,"%s    while(paction){\n",row);
    sprintf(row,"%s        paction->notify(p%s, event, paction->object, context);\n",row,name);
    sprintf(row,"%s        paction = paction->next;\n",row);
    sprintf(row,"%s    }\n",row);
    sprintf(row,"%s    return 0;\n",row);
    sprintf(row,"%s}\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"static struct %s_operation %s_op =\n{\n",name,name);
    sprintf(row,"%s    .init = %s_init,\n",row,name);
    sprintf(row,"%s    .release = %s_release,\n",row,name);
    sprintf(row,"%s    .register_notify = %s_register_notify,\n",row,name);
    sprintf(row,"%s    .unregister_notify = %s_unregister_notify,\n",row,name);
    sprintf(row,"%s    .trigger_notify = %s_trigger_notify,\n",row,name);
    sprintf(row,"%s};\n\n",row);
    fwrite(row,strlen(row),1,fp);

    sprintf(row,"%sint32_t create_init_%s(struct %s** p%s)\n","",name,name,name);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    int32_t ret;\n",row);
    sprintf(row,"%s    struct %s* ptmp;\n",row,name);
    sprintf(row,"%s    (*p%s) = (struct %s*)malloc(sizeof(struct %s));\n",row,name,name,name);

    sprintf(row,"%s    if(!(*p%s)){\n",row,name);
    sprintf(row,"%s        DBG(DBG_ERR,\"malloc error\\n\");\n",row);
    sprintf(row,"%s        return -ENOMEM;\n",row);
    sprintf(row,"%s    }\n",row);

    sprintf(row,"%s    ptmp = *p%s;\n",row,name);
    sprintf(row,"%s    memset(ptmp,0,sizeof(struct %s));\n",row,name);
    sprintf(row,"%s    ptmp->op = &%s_op;\n",row,name);
    sprintf(row,"%s    ret = ptmp->op->init(ptmp);\n",row);

    sprintf(row,"%s    if(ret < 0){\n",row);
    sprintf(row,"%s        DBG(DBG_ERR,\"init error\\n\");\n",row);
    sprintf(row,"%s        release_destroy_%s(ptmp);\n",row,name);
    sprintf(row,"%s        return ret;\n",row);
    sprintf(row,"%s    }\n",row);

    sprintf(row,"%s    return 0;\n",row);
    sprintf(row,"%s}\n\n",row);
    fwrite(row,strlen(row),1,fp);


    sprintf(row,"%svoid release_destroy_%s(struct %s* p%s)\n","",name,name,name);
    sprintf(row,"%s{\n",row);
    sprintf(row,"%s    if(p%s){\n",row,name);
    sprintf(row,"%s        p%s->op->release(p%s);\n",row,name,name);
    sprintf(row,"%s        free(p%s);\n",row,name);
    sprintf(row,"%s    }\n",row,name,name);
    sprintf(row,"%s}\n\n",row);
    fwrite(row,strlen(row),1,fp);

    fclose(fp);
    return 0;
}

int main(int argc,char* argv[])
{
    if(argc < 2){
        printf("Please input generate class name !\n");
        return -EINVAL;
    }
    if(access(GENERATE_PATH,R_OK)){
        if(mkdir(GENERATE_PATH,0777)<0){
            printf("create %s path error\n",GENERATE_PATH);
            return -EFAULT;
        }
    }
    if(generate_h(argv[1]) < 0){
        printf("generate h file error\n");
    }
    if(generate_c(argv[1]) < 0){
        printf("generate c file error\n");
    }
    return 0;
}
