#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <list>

using namespace std;
#define N_CHAR 128
typedef map<char,string> HuffTable;

/*
 * Huffman Tree
 */

struct HuffChar{
  char ch;
  int weight;
  HuffChar (char c = '^', int w = 0 ): ch ( c ), weight(w){};
  bool operator<(HuffChar const&hc){
    return weight >hc.weight;
  }
  bool operator == (HuffChar const&hc){
    return weight == hc.weight;
  }
};

/*
 * Huffman Tree
 */

struct HuffTreeNode{
  HuffChar data;
  HuffTreeNode *left;
  HuffTreeNode *right;
  HuffTreeNode():data(HuffChar()),left(NULL),right(NULL){};
  HuffTreeNode(HuffChar data):data(data),left(NULL),right(NULL){};
};

typedef list<HuffTreeNode* > HuffForest;


/*
 * Count the frequency of the chars in the file.
 */

int* statistics (const char* file){
  int* freq = new int[N_CHAR];
  memset(freq,0,sizeof(int)*N_CHAR);
  FILE* fp = fopen(file,"r");
  for(char ch;0<fscanf(fp,"%c",&ch);){
    freq[ch-0x00]++;
  }
  fclose(fp);
  return freq;
}

/*
 * Initiate the huffman forest.
 */

HuffForest* initForest(int*freq){
  HuffForest* forest = new HuffForest;
  for(int i = 0; i < N_CHAR;i++){
    if(freq[i]!= 0){
      forest->push_back(new HuffTreeNode());
      forest ->back()->data = (HuffChar(i+0x00,freq[i]));
    }
    //cout<<0x00+i<<'\t'<<freq[i]<<endl;
  }
  return forest;
}

/*
 * return the node which has the minimum frequency in the huffman tree.
 */

HuffTreeNode* minHChar(HuffForest* forest){
  HuffForest::iterator p = forest->begin();
  HuffForest::iterator minCharIt = p;
  HuffTreeNode* minChar = *p;
  int minWeight = (*p)->data.weight;
  while((++p)!=forest->end()){
    if(minWeight > (*p)->data.weight){
      //cout<<minWeight<<'\t'<<(*p)->data.weight<<endl;
      minWeight = (*p)->data.weight;
      minCharIt = p;
      minChar = *p;
    }
  }
  forest->erase(minCharIt);
  //cout<<"per_TIME"<<'\t'<<forest->size()<<endl;
  //cout<<minChar ->data.ch<<endl;
  return minChar;
}

/*
 * Generate the huffman tree.
 */

HuffTreeNode* generateTree(HuffForest* forest){
  while(1 < forest->size()){
    HuffTreeNode* T1 = minHChar(forest);
    HuffTreeNode* T2 = minHChar(forest);
    HuffTreeNode* S = new HuffTreeNode(HuffChar('^',T1->data.weight+T2->data.weight));
    S->left = T1;
    S ->right = T2;
    forest->push_back(S);
  }
  return forest->front();
}

/*
 * Generate the code table.
 */

void generateCT(string code,HuffTable* table,HuffTreeNode* root){
  if(root->left==NULL && root->right==NULL){
    //cout<<root->data.ch - 0x00 <<'\t'<<code<<endl;
    (*table)[root->data.ch] = code;
    //cout<<"length:"<<length<<endl;
  }
  if(root->left!=NULL){
    string subCode1 = code;
    subCode1.append("0");
    generateCT(subCode1,table,root->left);
  }
  if(root->right!=NULL){
    string subCode2 = code;
    subCode2.append("1");
    generateCT(subCode2,table,root->right);
  }
}

HuffTable* generateTable(HuffTreeNode* root){
  HuffTable*table = new HuffTable;
  string code;
  generateCT(code,table,root);
  return table;
}

unsigned char StrToBin(string str){
  int a = atoi(str.c_str());
  int b = 1;
  int ans = 0;
  while(a != 0){
    ans += a%10 * b;
    b *= 2;
    a /= 10;
  }
  return (unsigned char)ans;
}

string BinToStr(unsigned char c){
  string ans;
  while(c != 0){
    ans.insert(ans.begin(),c%2 + '0');
    c /= 2;
  }
  if(ans.length() < 8){
    ans.insert(ans.begin(), 8-ans.length(), '0');
  }
  return ans;
}

void compress(string inputFilename, string outputFilename) {
  int* freq = statistics(inputFilename.c_str());
  HuffForest* forest = initForest(freq);
  HuffTreeNode* root = generateTree(forest);
  HuffTable* table = generateTable(root);

  FILE* fin = fopen(inputFilename.c_str(),"r");
  FILE * fout = fopen(outputFilename.c_str(),"w");
  rewind(fin);

/*-----------------------------------
 *             CodeTable
 *-----------------------------------
 * 
 *             INCLUDING:
 *      [The number of extra zero]      
 *        [The number of chars]
 *[The chars included in the the file]
 *  [The length of code of each char]
 *          [codes of chars]
 *
 *------------------------------------
 *               Body
 *------------------------------------            
 */
  string codeString;
  //cout<<"charNum:"<<'\t'<<table->size()<<endl;
  codeString += BinToStr(0x00+table->size()); //DONE The number of chars.
  int count = 0;
  while(count<128){
  	if(table->count(0x00+count)!=0){
  		codeString += BinToStr(0x00+count); //done
  		codeString += BinToStr((*table)[0x00+count].length() + 0x00);//done
  		codeString += (*table)[0x00+count];
  		//cout<<0x00+count<<'\t'<<(*table)[0x00+count].length()<<'\t'<<(*table)[0x00+count]<<endl;
  	}
  	count++;
  }

  cout<<"CodeTable Length:"<<codeString.length()-8<<endl;
  int length1 = codeString.length();
  //cout<<codeString.length()<<endl;
  for(char ch;0<fscanf(fin,"%c",&ch);){
    codeString += (*table)[ch];
  }
  cout<<"body length:"<<codeString.length()-length1<<endl;

  int length2 = codeString.length();
  cout<<"length:"<<length2-length1<<endl;

  //cout<<codeString.length()<<endl;
  int zeroNum = 8 - (codeString.length()%8);
  cout<<"zeroNum:"<<BinToStr(zeroNum+0x00)<<'\t'<<zeroNum<<endl;
  cout<<codeString<<endl<<endl<<endl;
  for(int i = 0;i<zeroNum;i++)codeString += '0';   //DONE Extra zero
  //cout<<codeString.length()<<endl;
  cout<<codeString<<endl;

  codeString = BinToStr(zeroNum+0x00) + codeString;  //DONE Extra zero
  cout<<"?:"<<codeString.length()<<endl;
  //cout<<codeString<<endl;
  while(codeString.length()>=8){
    fputc(StrToBin(codeString.substr(0,8)),fout);
    codeString.erase(0,8);
  }

  
  fclose(fin);
  fclose(fout);
};

void decompress(string inputFilename, string outputFilename) {
  FILE* fin = fopen(inputFilename.c_str(),"r");
  FILE * fout = fopen(outputFilename.c_str(),"w");
  rewind(fin);
  string codeString;
  for(unsigned char ch;0<fscanf(fin,"%c",&ch);){
    codeString += BinToStr(ch);
  }
  cout<<"Original:"<<codeString.length()<<endl;
  
  int extraZero = StrToBin(codeString.substr(0,8))-0x00;
  cout<<"extraZero:"<<codeString.substr(0,8)<<'\t'<<extraZero<<endl;
  codeString.erase(0,8);
  //cout<<codeString<<endl<<endl<<endl;
  cout<<"pos:"<<codeString.length()-extraZero<<'\t'<<codeString.length()<<'\t'<<extraZero<<endl;
  cout<<codeString.substr(codeString.length()-extraZero,extraZero);
  codeString.erase(codeString.length()-extraZero,extraZero);
  //cout<<codeString<<endl<<endl<<endl;
  cout<<"After erasing zero:"<<codeString.length()<<endl;

  int charNum = StrToBin(codeString.substr(0,8))-0x00;
  codeString.erase(0,8);
  cout<<"After reading:"<<codeString.length()<<endl;

  //debug
  //int length1 = codeString.length();
  int length1 = codeString.length();
  map<string,char> codeTable;
  //cout<<"OK1"<<endl;
  //cout<<"charNum:"<<'\t'<<charNum<<endl;
  for(int i = 0;i<charNum;i++){
  	char ch = StrToBin(codeString.substr(0,8));
  	codeString.erase(0,8);

  	int length = StrToBin(codeString.substr(0,8))-0x00;
  	codeString.erase(0,8);

  	string code = codeString.substr(0,length);
  	codeString.erase(0,length);

  	codeTable[code] = ch;
  	//cout<<ch-0x00<<'\t'<<length<<'\t'<<code<<endl;
  }
  cout<<"CodeTable Length:"<<length1-codeString.length()<<endl;
  //cout<<"OK2"<<endl;
  //cout<<length1 - codeString.length()<<endl;
  cout<<"length:"<<codeString.length()<<endl;
  int index = 1;
  while(codeString!=""){
  	string code = codeString.substr(0,index);
  	//cout<<codeString.length()<<endl;
  	if(codeTable.count(code)!=0){
  		fputc(codeTable[code],fout);
  		codeString.erase(0,index);
  		index = 1;
  	}
  	else index++;
  	//cout<<index<<endl;
  }


  fclose(fin);
  fclose(fout);
}

void useage(string prog) {
  cerr << "Useage: " << endl
      << "    " << prog << "[-d] input_file output_file" << endl;
  exit(2);
}

int main(int argc, char* argv[]) {
  int i;
  string inputFilename, outputFilename;
  bool isDecompress = false;
  for (i = 1; i < argc; i++) {
    if (argv[i] == string("-d")) isDecompress = true;
    else if (inputFilename == "") inputFilename = argv[i];
    else if (outputFilename == "") outputFilename = argv[i];
    else useage(argv[0]);
  }
  if (outputFilename == "") useage(argv[0]);
  if (isDecompress) decompress(inputFilename, outputFilename);
  else compress(inputFilename, outputFilename);
  return 0;
}
