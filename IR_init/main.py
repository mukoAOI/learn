import bool_cal
import os
import index
word_index=index.read_index()

a=input("请输入正确的语句:")

import order_cal

print("倒排索引",order_cal.process_symbol(a,word_index))
b=order_cal.process_symbol(a,index.inderect(a))
print("暴力检索",b)

indexx=index.get_fileindex()

for i in b:
    print (indexx[i])



