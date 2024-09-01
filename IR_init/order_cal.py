from bool_cal import bool_and, bool_or,bool_not
from stack import Stack
from nltk import word_tokenize
import index
outsize=[',', '’','.', ':', ';', '?', '(', ')', '[', ']', '&', '!', '*', '@', '#', '$', '%']
def word_exist(word,index):
    if word == 'AND' or word == 'OR' or word=="NOT":
        return True
    elif word not in index:
        return False
    else:
        return True


def process_symbol(sentence,index_list):
    file_index=[i for i in range(len(index.get_fileindex()))]
    tokens = [i for i in word_tokenize(sentence) if i not in outsize]
    word_list = Stack()
    symbol_list = Stack()
    for word in tokens:
        if not word_exist(word,index_list):
            now_list=[]
        elif not(word == 'AND' or word == 'OR' or word=="NOT"):
            now_list = index_list[word]

        if word == 'AND' or word == 'OR' or word=="NOT":
            symbol_list.push(word)
        else:
            if (symbol_list.top()=="NOT"):
                word_list.push(bool_not(file_index,now_list))
                symbol_list.pop()
            else:
                word_list.push(now_list)

            if(symbol_list.top() == 'AND'):
                list_1 = word_list.pop()
                list_2 = word_list.pop()
                list_result = bool_and(list_1,list_2)
                word_list.push(list_result)
                symbol_list.pop()
            if(symbol_list.top() == 'OR'):
                list_1 = word_list.pop()
                list_2 = word_list.pop()
                list_result = bool_or(list_1, list_2)
                word_list.push(list_result)
                symbol_list.pop()
    return word_list.pop()