#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <limits.h>
typedef char* Word;
typedef struct code {
    Word** cmds;
    int len;
    int ncmds;
    int redirflag1;
    int redirflag2;
    int redirflag3;
    char* inputfile;
    char* outputfile;
    int backflag;
    int convflag;
} Code;
typedef struct string {
    Word *words;
    int len;
    int nlines;
} String;
typedef struct oneword {
    Word word;
    int len;
    int nchapter;
} Oneword;
#define EXIT_FLAG -2
#define CD_FLAG 1
#define EXEC_ERROR -1
#define NOREDIR_FLAG -3
#define CONV_REDIR_ERROR -4
/* Инициализация кода, строки, слова */
void intcode(Code* a){
    (*a).cmds = (Word**)malloc(sizeof(Word*));
    (*a).ncmds = 1;
    (*a).cmds[0] = NULL;
    (*a).len = 1;
    (*a).redirflag1=0;
    (*a).redirflag2=0;
    (*a).redirflag3=0;
    (*a).inputfile=NULL;
    (*a).outputfile=NULL;
    (*a).backflag=0;
    (*a).convflag=0;
}
void intstring(String* a){
    (*a).words = (Word*)malloc(sizeof(Word));
    (*a).nlines = 1;
    (*a).words[0] = NULL;
    (*a).len = 1;
}
void intword(Oneword* a){
    (*a).word = (Word)malloc(sizeof(char));
    (*a).nchapter = 1;
    (*a).word[0] = '\0';
    (*a).len = 1;
}
/* Функция расширения динамического массива из ссылок на строки, слова, символы, возвращает новый объем буфера */
int expandcode(Word*** a,int length){
    Word** b = (Word**)malloc(sizeof(Word*)*length*2);
    memmove(b, *a, sizeof(Word*)*length);
    free(*a);
    *a=b;
    return length*2;
}
int expand(Word** a, int length){
    Word* b = (Word*)malloc(sizeof(Word)*length*2);
    memmove (b, *a, sizeof(Word)*length);
    free(*a);
    *a = b;
    return length*2;
}
int expandword(Word* a, int length){
    Word b = (Word)malloc(sizeof(char)*length*2);
    strcpy(b, *a);
    free(*a);
    *a = b;
    return length*2;
}
void newchar(Oneword* s, int c, int i){
    if ((*s).nchapter+2 >= (*s).len) {
        (*s).len = expandword(&(*s).word,(*s).len);
    }
    (*s).word[i] = c;
    (*s).word[i + 1] = '\0';
    (*s).nchapter++;
}
void newchar1(Oneword** s, int c, int i){
    if ((**s).nchapter+2 >= (**s).len) {
        (**s).len = expandword(&(**s).word,(**s).len);
    }
    (**s).word[i] = c;
    (**s).word[i + 1] = '\0';
    (**s).nchapter++;
}
int newcmd(Word* a, Code* b, int j, int i){
    Word* acopy;
    acopy = (Word*)malloc(sizeof(Word)*(i+1));
    memcpy(acopy,a,sizeof(Word)*(i+1));
    if ((*b).ncmds+2 >= (*b).len){
        (*b).len = expandcode(&((*b).cmds),(*b).len);
    }
    (*b).cmds[j] = acopy;
    (*b).cmds[j+1]=NULL;
    (*b).ncmds++;
    j++;
    return j;
}
int onequotes(Oneword* s, int* i){
    int j, c, m, l=1;
    for (j=0;((c = getchar()) != EOF) && c != '\n' && c!='\''; j++){
        newchar1(&s,c,j);
    }
    *i = j;
    if (c == '\''){
        for(l=0; ((m = getchar()) != EOF) && m != '\n' && m != ' ' && m != '>' && m!= '<' && m!='|' && m!='&'; l++){}
        if (l!=0){
            printf("Command not found1\n");
            return -1;
        }
        return m;
    }
    if(c == '\n' || c == EOF){
        printf("Command not found2\n");
        return -1;
    }
    return c;
}
int twoquotes (Oneword* s, int* i){
    int j,c, c1='\"',m,l;
    for(j=0; ((c = getchar()) != EOF) && c != '\n'; j++){
        //Экранирование
        if (c1 == '\\' && (c == '\'' || c == '\"')){
            j--;
        }
        //Вторая кавычка
        if (c1 != '\\' && c == '\"'){
            if ((*s).nchapter+1 >= (*s).len) {
                (*s).len = expandword(&(*s).word,(*s).len);
            }
            (*s).word[j] = '\0';
            (*s).nchapter++;
            for(l=0; ((m = getchar()) != EOF) && m != '\n' && m != ' ' && m != '>' && m!= '<' && m!='|' && m!='&'; l++){}
            if (l!=0){
                printf("Command not found3\n");
                return -1;
            }
            *i=j;
            return m;
        }
        newchar1(&s,c,j);
        c1=c;
    }
    *i=j;
    if(c == '\n' || c == EOF){
        printf("Command not found4\n");
        return -1;
    }
    return c;
}

int background(int c, Code* b ){
    if (c=='&'){
        ((*b).backflag)++;
    }
    if ((*b).backflag>1){
        printf("Command not found5\n");
        return -1;
    }
    return 0;
}
/* Функция чтения строки, возвращает последнюю букву слова слова, indicator - символ, по которому произошел выход */
int getword (char** index, int* indicator) {
    Oneword s;
    intword(&s);
    int i;
    int c, m;
    for (i=0; ((c = getchar()) != EOF) && c != '\n' && c != ' ' && c != '>' && c!= '<' && c!='|' && c!='&'; i++) {
        //Обработка одинарных кавычек
        if (c=='\''){
            if(i==0){
                c = onequotes(&s, &i);
                if (c==-1){
                    return -1;
                }else{break;}
            }else{
                for(; ((m = getchar()) != EOF) && m != '\n' && m != ' '; ){}
                printf("Command not found6\n");
                return -1;
            }
        }
        //Обработка двойных кавычек
        if (c=='\"'){
            if(i==0){
                c = twoquotes(&s, &i);
                if (c==-1){
                    return -1;
                }else{break;}
            }else{
                for(; ((m = getchar()) != EOF) && m != '\n' && m != ' '; ){}
                printf("Command not found7\n");
                return -1;
            }
        }
        //Обычная обработка слова
        newchar(&s,c,i);
    }
    *indicator = c;//по какому символу произошел выход из цикла
    if (i!=0) {
        *index = s.word;
        //обработка \\n
        if (s.word[i - 1] == '\\' && c == '\n') {
            s.word[i - 1] = '\0';
            return '\\';
        } else {
            return s.word[i - 1];
        }
    }else{
        free(s.word);
        return -2;
    }
}
void printcmd(Word* b){
    //printf("Cmd:\n");
    for (int k=0; b[k]!=NULL; k++) {
        printf("Word: %s", b[k]);
        printf(" ");
    }
    printf("\n");
}
int redirection1(Code* b, int* indicator){
    int c = *indicator;
    int endchar;
    if (c=='>'){
        (*b).redirflag1++;
    }
    if((*b).redirflag1==1){
        endchar = getword(&((*b).outputfile),indicator);
        if (endchar==-2 && *indicator=='>'){
            (*b).redirflag2++;
            (*b).redirflag1=0;
            free((*b).outputfile);
            endchar = getword(&((*b).outputfile),indicator);
        }
        if (endchar==-2 && *indicator==' '){
            free((*b).outputfile);
            endchar = getword(&((*b).outputfile),indicator);
        }
    }
    if((*b).redirflag1>1 || (*b).redirflag2>1){
        printf("Command not found8\n");
        return -1;
    }
    return 0;
}
int redirection2(Code* b, int* indicator){
    int c = *indicator;
    int endchar;
    if (c=='<'){
        (*b).redirflag3++;
    }
    if((*b).redirflag3==1){
        endchar = getword(&((*b).inputfile),indicator);
        if (endchar==-2 && *indicator==' '){
            free((*b).inputfile);
            endchar = getword(&((*b).inputfile),indicator);
        }
    }
    if ((*b).redirflag3>1){
        printf("Command not found9\n");
        return -1;
    }
    return 0;
}

int getcmd(int* indicator, Code* b){
    int i=0, j=0;
    int  endchar;
    String a;
    intstring(&a);
    do {
        endchar = getword((a.words+i), indicator);
        if (endchar == -1){return -1;}
        if (*indicator=='>'){
            if(redirection1(b, indicator)==-1){return -1;}
        }
        if (*indicator=='<'){
            if(redirection2(b, indicator)==-1){return -1;}
        }
        if (endchar != -2){
            if (a.nlines + 2 >= a.len) {
                a.len = expand(&a.words, a.len);
            }
            a.words[i+1] = NULL;
            a.nlines++;
            i++;
        } //если слово пустое(повторяющиеся пробелы)-пропускаем его
        if(background(*indicator, b)==-1){return -1;}
        if (*indicator=='|'){
            (*b).convflag++;
            j = newcmd(a.words,b,j,i);
            free(a.words);
            i=0;
            intstring(&a);
        }
    }
    while (*indicator!=EOF && *indicator != '\n' || *indicator == '\n' && endchar == '\\');
    if (i!=0){
        j = newcmd(a.words,b,j,i);
        free(a.words);
        return 0;
    }else{
        free(a.words);
        return -1;
    }

}
void freememory(Word** a){
    for (int k=0; a[k]!=NULL; k++){
        for (int n=0; a[k][n]!=NULL; n++) {
            free (a[k][n]);
        }
        free (a[k]);
    }
    free (a);
}
void freememory1(Code b){
    freememory(b.cmds);
    free(b.outputfile);
    free(b.inputfile);
}
int hello(){
    int pathlen = PATH_MAX;
    Word name = (Word)malloc(pathlen*sizeof(char));
    name = getcwd(name,pathlen*sizeof(char));
    printf("kariya@kariya-Inspiron-5567:~%s$ ",name);
    free(name);
    return 0;
}
int cdcmd(Word* cmd){
    Word s = "cd";
    int i = strcmp(cmd[0],s);
    if (i==0){
        if(cmd[1]==NULL){
            printf ("No arguments!\n");
            return -1;
        }else{
            if(chdir(cmd[1])!=0){
                printf("Error chdir!\n");
                return -1;
            }else{
                return 1;
            }
        }
    }else{return 0;}
}
int exitcmd(Word* cmd){
    Word s = "exit";
    if(cmd[0]!=NULL){
        int i = strcmp(cmd[0],s);
        if(i==0){
            return 1;
        }
        return 0;
    }
    return 0;
}
int dosmth1(Word* cmd){
    int noexit,nocd;
    noexit=exitcmd(cmd);
    if(noexit==0){
        nocd=cdcmd(cmd);
        if (nocd==-1){return -1;}
        if(nocd==0){
            execvp(cmd[0],cmd);
            return EXEC_ERROR;
        }else{return CD_FLAG;}
    }else{return EXIT_FLAG;}
}
int textdir(Code b, int i){
    int err=NOREDIR_FLAG;
    if((b.redirflag1==1 || b.redirflag2==1) && (b.convflag==0 || i==b.convflag)){
        int saveStd1 = dup(1);
        int fdes1;
        if(b.redirflag1==1){
            fdes1 = open(b.outputfile,O_CREAT|O_TRUNC|O_WRONLY,0666);
            dup2(fdes1,1);
            close(fdes1);
        }
        if(b.redirflag2==1){
            fdes1 = open(b.outputfile,O_CREAT|O_APPEND|O_WRONLY,0666);
            dup2(fdes1,1);
            close(fdes1);
        }
        if(b.convflag>0 || b.redirflag3==0){
            err=dosmth1(b.cmds[i]);
            dup2(saveStd1,1);
            close(saveStd1);
            return err;
        }
        if(b.redirflag3==1){
            int saveStd2 = dup(0);
            int fdes2 = open(b.inputfile,O_RDONLY,0666);
            dup2(fdes2,0);
            close(fdes2);
            err=dosmth1(b.cmds[i]);
            dup2(saveStd1,1);
            close(saveStd1);
            dup2(saveStd2,0);
            close(saveStd2);
            return err;
        }
    }
    if(b.redirflag3==1 && (b.convflag==0 || i==0)){
        int saveStd2 = dup(0);
        int fdes2 = open(b.inputfile,O_RDONLY,0666);
        dup2(fdes2,0);
        close(fdes2);
        err=dosmth1(b.cmds[i]);
        dup2(saveStd2,0);
        close(saveStd2);
        return err;
    }
    return NOREDIR_FLAG;
}
int conv(Code b){
    int pid, err1;
    int* PidMass = (int*)malloc((b.convflag+1)*sizeof(int));
    int** DesMass = (int**)malloc(b.convflag*sizeof(int*));
    for(int i=0; i<=b.convflag; i++){
        PidMass[i]=0;
    }
    for(int i=0; i < b.convflag; i++){
        int fdes[2];
        DesMass[i]=fdes;
        pipe(DesMass[i]);
    }
    for(int i=0; i<=b.convflag; i++){
        if((pid=fork())==0){
            //если не первая команда
            if(i!=0){
                dup2(DesMass[i-1][0],0);
            }
            //если не последняя
            if(i!=b.convflag) {
                dup2(DesMass[i][1],1);
            }
            for(int j=0; j<b.convflag; j++){
                close(DesMass[j][0]);
                close(DesMass[j][1]);
            }
            //перенаправление ввода-вывода?
            err1=textdir(b,i);
            //выполнение команды
            //команда не распознана(exit или cd в перенаправлении)
            if((err1==CD_FLAG)||(err1==EXEC_ERROR)||(err1==EXIT_FLAG)||(err1==CONV_REDIR_ERROR)){
                return -1;
            }
            //если не было перенаправления
            if(err1==NOREDIR_FLAG){
                dosmth1(b.cmds[i]);
                return -1;
            }
        }else{
            if(pid>0){
                PidMass[i]=pid;
            }
        }
    }
    for(int j=0; j<b.convflag; j++){
        close(DesMass[j][0]);
        close(DesMass[j][1]);
    }
    if(b.backflag==0){
        for (int j=0; j<=b.convflag; j++){
            if(PidMass[j]>0) {
                waitpid(PidMass[j], NULL, 0);
            }
        }
    }
    free(PidMass);
    free(DesMass);
    return 0;
}
int main() {
    Code b;
    int indicator = 'a',pid, err0, err, err1;
    while(indicator!=EOF){
        hello();
        indicator = 'a';
        intcode(&b);
        //если считали команду без ошибок
        if((getcmd(&indicator,&b))!= -1){
            if(b.convflag>0){
                err0=conv(b);
                while(waitpid(-1,0,WNOHANG)>0){};
                freememory1(b);
                if(err0==-1){
                    break;
                }
                continue;
            }
            if((pid=fork())==0){
                err=textdir(b,0);
                if(err==NOREDIR_FLAG){
                    if((err1=dosmth1(b.cmds[0]))==-2){
                        freememory1(b);
                        break;
                    }
                    if(err1==EXEC_ERROR){
                        printf("Command not found!\n");
                        freememory1(b);
                        continue;
                    }
                }else{
                    if(err==EXIT_FLAG || err==CD_FLAG || err==EXEC_ERROR){
                        printf("Command not found11\n");
                        freememory1(b);
                        continue;
                    }
                }
            }else{
                if(pid>0 && b.backflag==0){
                    waitpid(pid,NULL,0);
                }
            }
        }
        while(waitpid(-1,0,WNOHANG)>0){};
        freememory1(b);
    }
    //while(waitpid(-1,0,WNOHANG)!=-1){};
    return 0;
}