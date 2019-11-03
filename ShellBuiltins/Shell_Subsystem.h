%token &lt;string_val&gt; WORD
%token NOTOKEN NEWLINE GREAT GREATAMP GREATGREAT GREATGREATAMP LESS AMPERSAND
PIPE
%union {

char *string_val;
}
%{
//#define yylex yylex
#include &lt;stdio.h&gt;
#include &lt;stdlib.h&gt;
#include &lt;string.h&gt;
#include &lt;sys/types.h&gt;
#include &lt;regex.h&gt;
#include &lt;dirent.h&gt;
#include &lt;assert.h&gt;
#include &quot;command.h&quot;
#include &lt;pwd.h&gt;
#define MAXFILENAME 1024

char **array = NULL;
int nEntries = 0;
int maxEntries = 20;
void yyerror(const char * s);
void expandWildcardsIfNecessary(char * arg);
void expandWildcard(char * prefix, char *suffix);
int compare(const void *str1, const void *str2);
int yylex();
%}
%%
goal:
commands
;
commands:
command
| commands command
;
command: simple_command
;
simple_command:
pipe_list iomodifier_list background_opt NEWLINE {
Command::_currentCommand.execute();
}
| NEWLINE { Command::_currentCommand.prompt(); }
| error NEWLINE { yyerrok; }
;
pipe_list:
pipe_list PIPE command_and_args
| command_and_args
;
command_and_args:
command_word arg_list {
Command::_currentCommand.
insertSimpleCommand( Command::_currentSimpleCommand );

}
;
arg_list:
arg_list argument
|
;
argument:
WORD {
expandWildcardsIfNecessary($1);
}
;
command_word:
WORD {
Command::_currentSimpleCommand = new SimpleCommand();
Command::_currentSimpleCommand-&gt;insertArgument( $1 );
}
;

iomodifier_list:
iomodifier_list iomodifier_opt
|
;
iomodifier_opt:
GREAT WORD {
Command::_currentCommand._outFile = $2;
Command::_currentCommand._outCount++;
}
|
GREATGREAT WORD {
Command::_currentCommand._append = 1;
Command::_currentCommand._outFile = $2;
Command::_currentCommand._outCount++;
}
|
GREATAMP WORD {
Command::_currentCommand._errFile = $2;
Command::_currentCommand._errCount++;
}
|
GREATGREATAMP WORD {
Command::_currentCommand._append = 1;
Command::_currentCommand._errFile = $2;
Command::_currentCommand._errCount++;
}
|
LESS WORD {
Command::_currentCommand._inputFile = $2;
Command::_currentCommand._inCount++;
}
;
background_opt:
AMPERSAND {
Command::_currentCommand._background = 1;
}
|
;
%%
void
yyerror(const char * s)
{
fprintf(stderr,&quot;%s&quot;, s);
}
void expandWildcardsIfNecessary(char * arg)
{
char *tempArg = strdup(arg);
if (strchr(tempArg, &#39;*&#39;) == NULL &amp;&amp; strchr(tempArg, &#39;?&#39;) == NULL &amp;&amp; strchr(tempArg, &#39;~&#39;) == NULL)
{
Command::_currentSimpleCommand-&gt;insertArgument(tempArg);
return;
}
expandWildcard(&quot;&quot;, tempArg);
if(nEntries &gt; 1)
qsort(array, nEntries, sizeof(char*), compare);
for (int i = 0; i &lt; nEntries; i++)
{
Command::_currentSimpleCommand-&gt;insertArgument(array[i]);
}

if(array != NULL)
free(array);
array = NULL;
nEntries = 0;
return;
}
void expandWildcard(char * prefix, char *suffix)
{
if (suffix[0]== 0) {
return;
}
bool dirFlag = false;
if(suffix[0] == &#39;/&#39;)
{
dirFlag = true;
}
char * suffix_backup = suffix;
char * s = strchr(suffix, &#39;/&#39;);
char component[MAXFILENAME];
for(int i = 0; i &lt; MAXFILENAME; i++)
component[i] = 0;
if (s != NULL){
if(prefix[0] == 0 &amp;&amp; dirFlag)
{
suffix = s+1;
s = strchr(suffix, &#39;/&#39;);
if(s == NULL)
{
strcpy(component, suffix);
suffix = suffix + strlen(suffix);
prefix = &quot;/&quot;;
}
else
{
strncpy(component,suffix, s-suffix);
suffix = s + 1;
prefix = &quot;/&quot;;
}
}
else
{
strncpy(component,suffix, s-suffix);
suffix = s + 1;
}
}
else {
strcpy(component, suffix);
suffix = suffix + strlen(suffix);
}
char tildePrefix[MAXFILENAME];
for(int i = 0; i &lt; MAXFILENAME; i++)
tildePrefix[i] = 0;
if(component[0] == &#39;~&#39;)
{
struct passwd *pwd;
if(strcmp(component, &quot;~&quot;) == 0)
pwd = getpwnam(getenv(&quot;USER&quot;));

else
pwd = getpwnam(component+1);
if(pwd == NULL)
printf(&quot;Could not access user %s.\n&quot;, component+1);
else
{
if(suffix[0] == 0 &amp;&amp; prefix[0] == 0)
sprintf(tildePrefix,&quot;%s&quot;,pwd-&gt;pw_dir);
else if(suffix[0] == 0)
sprintf(tildePrefix,&quot;%s/%s&quot;,pwd-&gt;pw_dir, component);
else if(prefix[0] == 0)
sprintf(tildePrefix,&quot;%s/%s&quot;,pwd-&gt;pw_dir, suffix);
expandWildcardsIfNecessary(tildePrefix);
return;
}
}
char newPrefix[MAXFILENAME];
for(int i = 0; i &lt; MAXFILENAME; i++)
newPrefix[i] = 0;
if (strchr(component, &#39;*&#39;) == NULL &amp;&amp; strchr(component, &#39;?&#39;) == NULL) {
if(prefix[0] == 0 &amp;&amp; !dirFlag)
sprintf(newPrefix,&quot;%s&quot;,component);
else if(strcmp(prefix, &quot;/&quot;) == 0)
sprintf(newPrefix,&quot;/%s&quot;,component);
else
sprintf(newPrefix,&quot;%s/%s&quot;,prefix,component);
expandWildcard(newPrefix, suffix);
return;
}
char * reg = (char*)malloc(2*strlen(component)+10);
char * a = component;
char * r = reg;
*r = &#39;^&#39;; r++;
while (*a)
{
if (*a == &#39;*&#39;) { *r=&#39;.&#39;; r++; *r=&#39;*&#39;; r++; }
else if (*a == &#39;?&#39;) { *r=&#39;.&#39;; r++;}
else if (*a == &#39;.&#39;) { *r=&#39;\\&#39;; r++; *r=&#39;.&#39;; r++;}
else { *r=*a; r++;}
a++;
}
*r=&#39;$&#39;; r++; *r=0;
regex_t re;
regmatch_t match;
int result = regcomp( &amp;re, reg, REG_EXTENDED|REG_NOSUB);
free(reg);
if (result != 0)
{
perror(&quot;Bad regular expression: compile&quot;);
return;
}
char * d;
if (prefix[0] == 0)
d = &quot;.&quot;;
else d = prefix;
DIR * dir = opendir(d);
if (dir == NULL)
{
return;
}

struct dirent * ent;
if(array == NULL)
array = (char**) malloc(maxEntries*sizeof(char*));
while ( (ent = readdir(dir))!= NULL) {
if (regexec(&amp;re, ent-&gt;d_name, 1, &amp;match, 0 ) == 0)
{
if(array != NULL &amp;&amp; nEntries &gt; 1)
qsort(array, nEntries, sizeof(char*), compare);
if (nEntries == maxEntries)
{
maxEntries *=2;
array = (char**) realloc(array, maxEntries*sizeof(char*));
assert(array!=NULL);
}
if (ent-&gt;d_name[0] == &#39;.&#39;)
{
if (component[0] == &#39;.&#39;)
{
if(prefix[0] == 0)
sprintf(newPrefix,&quot;%s&quot;,ent-&gt;d_name);
else if(prefix[0] == &#39;/&#39; &amp;&amp; prefix[1] == 0)
sprintf(newPrefix,&quot;/%s&quot;,ent-&gt;d_name);
else
sprintf(newPrefix,&quot;%s/%s&quot;,prefix,ent-&gt;d_name);
expandWildcard(newPrefix,suffix);
if(s == NULL)
{
array[nEntries]= strdup(newPrefix);
nEntries++;
}
}
}
else
{
if(prefix[0] == 0)
sprintf(newPrefix,&quot;%s&quot;,ent-&gt;d_name);
else if(prefix[0] == &#39;/&#39; &amp;&amp; prefix[1] == 0)
sprintf(newPrefix,&quot;/%s&quot;,ent-&gt;d_name);
else
sprintf(newPrefix,&quot;%s/%s&quot;,prefix,ent-&gt;d_name);
expandWildcard(newPrefix,suffix);
if(s == NULL)
{
array[nEntries]= strdup(newPrefix);
nEntries++;
}
}
}
}
closedir(dir);
return;
}
int compare(const void *str1, const void *str2)
{
return strcmp(*(char *const*)str1, *(char *const*)str2);

}
#if 0
main()
{
yyparse();
}
#endif
