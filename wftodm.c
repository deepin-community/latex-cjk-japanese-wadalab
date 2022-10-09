#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
char *charstrs[34][256];
int charlens[34][256];
char *version_str="001.001";
int kcode_high;
char *StdHW="32";
char *StdVW="32";
char *UniqueId="9876";
char *FontBase="dm";
char fontname[256];
FILE *ofp;
int ecol=0;

char *jsf_names[] = {
    "",
    "jsy",
    "jroma",
    "jhira",
    "jkata",
    "jgreek",
    "jrussian",
    "jkeisen",
    "jka",
    "jkb",
    "jkc",
    "jkd",
    "jke",
    "jkf",
    "jkg",
    "jkh",
    "jki",
    "jkj",
    "jkk",
    "jkl",
    "jkm",
    "jkn",
    "jko",
    "jkp",
    "jkq",
    "jkr",
    "jks",
    "jkt",
    "jku",
    "jkv",
    "jkw",
    "jkx",
    "jky",
    "jkz",
    NULL
};
#define	kushift(c)	c+0x20
#define	tenshift(c)	c+0x20
compute_fc(ku, ten, f, c)
register short ku, ten;
unsigned short *f, *c;
{
    register int n;

    ku -= 0x20;
    ten -= 0x20;
    *f = 1;
    *c = 1;
    if (ku <= 0 || (9 <= ku && ku <= 15) || ku > 84) {
	fprintf(stderr,"invalid ku in jis (%x, %x)", ku+0x20, ten+0x20);
	return;
    }
    if (ten < 1 || ten > 94) {
	fprintf(stderr,"invalid ten in jis (%x, %x)", ku+0x20, ten+0x20);
	return;
    }
    if (ku <= 8) {
	if (ku == 1) {
	    *f = 1;
	    *c = ten;
	} else if (ku == 2) {
	    *f = 1;
	    *c = ten+100;
	} else if (ku == 3) {
	    *f = 2;
	    *c = ten+32;
	} else {
	    *f = ku-1;
	    *c = ten;
	}
    } else if (ku <= 47) {	/* Daiich Suijun */
	n = (ku-16)*94+ten-1;
	*f = (n/256)+8;
	*c = n%256;
    } else {			/* Daini Suijun */
	n = (ku-48)*94+ten-1;
	*f = (n/256)+20;
	*c = n%256;
    }
}
readfiles(filename)
char *filename;
{
  FILE *fd;
  char buf[4096],*cptr;
  int kcode_low,i,len,kcode,c1;
  unsigned short f,c;

  if((fd=fopen(filename,"r"))==NULL){
    fprintf(stderr,"File %s is not found\n",filename);
    exit(1);
  }
  while(fgets(buf,4096,fd)!=NULL){
    len=strlen(buf);
    if(buf[0]=='<' && !strncmp(buf+len-6 ,"CompD",5)){
      kcode=strtol(buf+len-12,NULL,16);
      compute_fc((kcode>>8)&255, (kcode&255), &f, &c);
      charlens[f][c]=(len-16)/2;
      cptr=charstrs[f][c]=(char *)malloc((len-16)/2);
      for(i=0;i<(len-16)/2;i++){
	c1=buf[i*2+1];c=buf[i*2+2];
	*cptr++ = ((c1>='a' ? c1-'a'+10 : c1-'0')<<4)
	  |(c>='a' ? c-'a'+10 : c-'0');
      }
    }
  }
  fclose(fd);
}

main(ac,ag)
char **ag;
{
  int i,j;
  
  for(i=1;i<ac;i++){
    if(*ag[i]!='-')break;
    if(!strcmp(ag[i]+1,"version")){
      version_str=ag[++i];
    }
    else if(!strcmp(ag[i]+1,"StdHW")){
      StdHW=ag[++i];
    }
    else if(!strcmp(ag[i]+1,"StdVW")){
      StdVW=ag[++i];
    }
    else if(!strcmp(ag[i]+1,"UniqueId")){
      UniqueId=ag[++i];
    }
    else if(!strcmp(ag[i]+1,"FontBase")){
      FontBase=ag[++i];
    }
  }
  for(;i<ac;i++)
    readfiles(ag[i]);
  for(i=1;i<sizeof(jsf_names)/sizeof(char *)-1;i++){
    for(j=0;j<256;j++)
      if(charstrs[i][j])break;
    if(j<256){
      sprintf(fontname,"%s%s",FontBase,jsf_names[i]);
      output_pfa(i);
      output_afm(i);
    }
  }
}
output_afm(file)
{
  int i;
  char filename[256];
  
  sprintf(filename,"%s.afm",fontname);
  if((ofp=fopen(filename,"w"))==NULL){
    fprintf(stderr,"Failed opening file %s\n",filename);
    exit(1);
  }
  fprintf(ofp,"StartFontMetrics 2.0\n");
  fprintf(ofp,"FontName %s\n",fontname);
  fprintf(ofp,"FullName %s\n",fontname);
  fprintf(ofp,"FamilyName %s\n",FontBase);
  fprintf(ofp,"Weight Regular\n");
  fprintf(ofp,"ItalicAngle 0\n");
  fprintf(ofp,"IsFixedPitch true\n");
  fprintf(ofp,"FontBBox 0 0 1000 1000\n");
  fprintf(ofp,"UnderlinePosition 0\n");
  fprintf(ofp,"UnderlineThickness 0\n");
  fprintf(ofp,"Version 001.001\n");
  fprintf(ofp,"EncodingScheme DmEncoding\n");
  fprintf(ofp,"StartCharMetrics 256\n");
  for(i=0;i<256;i++)
    if(charstrs[file][i])
      fprintf(ofp,"C %d ; WX 1000 ; N c%02X ; B 0 0 1000 1000 ;\n",i,i);
  fprintf(ofp,"EndCharMetrics\n");
  fprintf(ofp,"EndFontMetrics\n");
  fclose(ofp);
}


output_header(file)
{
  int i,j;
  time_t now=time(0);
  fprintf(ofp,"%%!PS-AdobeFont-1.0: %s %s\n",fontname,version_str);
  fprintf(ofp,"%%%%Creation Date: %s",ctime(&now));
  fprintf(ofp,"%%%%VMusage: 100000 100000\n"); /* dummy */
  fprintf(ofp,"12 dict begin\n");
  fprintf(ofp,"/FontInfo 8 dict dup begin\n");
  fprintf(ofp,"/version (%s) readonly def\n",version_str);
  fprintf(ofp,"/FullName (%s) readonly def\n",fontname);
  fprintf(ofp,"/FamilyName (%s) readonly def\n",FontBase);
  fprintf(ofp,"/Weight (Regular) readonly def\n");
  fprintf(ofp,"/ItalicAngle 0 def\n");
  fprintf(ofp,"/isFixedPitch true def\n");
  fprintf(ofp,"/UnderlinePosition 0 def\n");
  fprintf(ofp,"/UnderlineThickness 0 def\n");
  fprintf(ofp,"end readonly def\n");
  fprintf(ofp,"/FontName /%s def\n",fontname);
  fprintf(ofp,"/DmEncoding [\n");
  for(i=0;i<16;i++){
    for(j=0;j<16;j++)
      fprintf(ofp," /c%02X",i*16+j);
    putc('\n',ofp);
  }
  fprintf(ofp,"] readonly def\n");
  fprintf(ofp,"/Encoding DmEncoding def\n");
  fprintf(ofp,"/PaintType 0 def\n");
  fprintf(ofp,"/FontType 1 def\n");
  fprintf(ofp,"/FontMatrix [.001 0 0 .001 0 -0.16] readonly def\n");
  fprintf(ofp,"/UniqueId %s def\n",UniqueId);
  fprintf(ofp,"/FontBBox [0 0 1000 1000] readonly def \n");
  fprintf(ofp,"currentdict end\n");
  fprintf(ofp,"currentfile eexec\n");
}
output_pfa(file)
{
  int i,j,len,charlen;
  /* new integer ii for counting characters */
  int ii=1;
  char filename[256];
  
  sprintf(filename,"%s.pfa",fontname);
  if((ofp=fopen(filename,"w"))==NULL){
    fprintf(stderr,"Failed opening file %s\n",filename);
    exit(1);
  }
  output_header(file);
  init_crypt();
  e_putchar(0);
  e_putchar(0);
  e_putchar(0);
  e_putchar(0);
  e_printf("dup /Private 17 dict dup begin\n");
  e_printf("/-|{string currentfile exch readstring pop}executeonly def\n");
  e_printf("/|-{noaccess def}executeonly def\n");
  e_printf("/|{noaccess put}executeonly def\n");
  e_printf("/BlueValues [] |-\n");
  e_printf("/OtherBlues [] |-\n");
  e_printf("/MinFeature{16 16} |-\n");
  e_printf("/StdHW [ %s ] |-\n",StdHW);
  e_printf("/StdVW [ %s ] |-\n",StdVW);
  e_printf("/ForceBold false def\n");
  e_printf("/password 5839 def\n");
  e_printf("/UniqueId %s def\n",UniqueId);
  e_printf("/OtherSubrs [] |-\n");
  e_printf("/Subrs 5 array\n");
  e_printf("dup 0 15 -| \020\2771p|\020\024\020=-\223D\\\342R |\n");
  e_printf("dup 1 9 -| \020\2771py\274\366Uz |\n");
  e_printf("dup 2 9 -| \020\2771py\275\304\236i |\n");
  e_printf("dup 3 5 -| \020\2771p\371 |\n");
  e_printf("dup 4 12 -| \020\2771p~\266+6\034\3446z |\n");
  e_printf("|-\n");
  for(i=0;i<256;i++){
    if(charstrs[file][i]!=NULL)
      ii++;
  }
  e_printf("2 index /CharStrings %i dict dup begin\n",ii);
  e_printf("/.notdef 10 -| \020\277\061\160\171\312\070\217\347\143 |-\n");
  for(i=0;i<256;i++){
    if(charstrs[file][i]!=NULL){
      e_printf("/c%02X %d -| ",i,charlens[file][i]);
      for(j=0;j<charlens[file][i];j++)
	e_putchar(*(charstrs[file][i]+j));
      e_printf(" |-\n");
    }
  }
  e_printf("end\n");
  e_printf("end\n");
  e_printf("readonly put\n");
  e_printf("noaccess put\n");
  e_printf("dup/FontName get exch definefont pop\n");
  e_printf("mark currentfile closefile\n");
  putc('\n',ofp);
  fprintf(ofp,"0000000000000000000000000000000000000000000000000000000000000000\n");
  fprintf(ofp,"0000000000000000000000000000000000000000000000000000000000000000\n");
  fprintf(ofp,"0000000000000000000000000000000000000000000000000000000000000000\n");
  fprintf(ofp,"0000000000000000000000000000000000000000000000000000000000000000\n");
  fprintf(ofp,"0000000000000000000000000000000000000000000000000000000000000000\n");
  fprintf(ofp,"0000000000000000000000000000000000000000000000000000000000000000\n");
  fprintf(ofp,"0000000000000000000000000000000000000000000000000000000000000000\n");
  fprintf(ofp,"0000000000000000000000000000000000000000000000000000000000000000\n");
  fprintf(ofp,"cleartomark\n");
  fclose(ofp);
}

e_printf(form,i0,i1,i2,i3,i4,i5,i6,i7,i8,i9)
char *form;
{
  int len,i;
  unsigned char buf[4096];
  sprintf(buf,form,i0,i1,i2,i3,i4,i5,i6,i7,i8,i9);
  len=strlen(buf);
  for(i=0;i<len;i++)
    e_putchar(buf[i]);
}
e_putchar(plain)
unsigned char plain;
{
  fprintf(ofp,"%02X",encrypt_char(plain));
  ecol+=2;
  if(ecol==64){putc('\n',ofp);ecol=0;}
}

unsigned short r,c1,c2;
init_crypt()
{
  r=55665,c1=52845,c2=22719;
  ecol=0;
}
encrypt_char(plain)
unsigned char plain;
{

  unsigned char cipher;
  cipher=(plain ^ (r>>8));
  r=(cipher+r)*c1+c2;
  return cipher;
}

