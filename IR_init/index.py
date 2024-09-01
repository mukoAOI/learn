import os
import re
from nltk import word_tokenize
outsize=[',', '’','.', ':', ';', '?', '(', ')', '[', ']', '&', '!', '*', '@', '#', '$', '%']
def create_index(path="input"):
    file_list=os.listdir(path)
    word_index={}
    num=0
    for i in file_list:
        with open(path+"/"+i,encoding="utf8") as f:
            word=[i for i in word_tokenize(f.read()) if i not in outsize]
            for i in word:
                if i not in word_index:
                    word_index[i]=[num]
                elif num not in word_index[i]:
                        word_index[i].append((num))
            num+=1

    return word_index

def write_index(word_index,wpath="out.txt"):
    with open(wpath,"w",encoding="utf8") as f:
        for word,file_num in word_index.items():
            f.write(word+" "+" ".join([str(i) for i in file_num])+"\n")

def read_index(opath="out.txt",ipath="input"):
    if (not (os.path.exists("out.txt"))):
        write_index(create_index(ipath)[0])
    word_index={}
    with open (opath,"r",encoding="utf8") as f:
        for i in f:
            i=i.strip()
            li=i.split(" ")
            word_index[li[0]]=[int (i ) for i in li[1:]]
    return word_index


def inderect(sent,path="input"):
    file_list = os.listdir(path)
    word = [i for i in word_tokenize(sent) if i not in outsize]
    word_list=[ww for ww in word if (ww!="AND" and ww!="OR" and ww!="NOT")]
    word_index = {i:[] for i in word_list}
    num=0
    for i in file_list:
        with open(path + "/" + i, encoding="utf8") as f:
            word = [i for i in word_tokenize(f.read()) if i not in outsize]
            for j in word_list:
                if j in word:
                    word_index[j].append(num)
        num+=1
    return word_index


def get_fileindex(path="input"):
    file_list = os.listdir(path)
    return {i:file_list[i] for i in range(len(file_list))}




