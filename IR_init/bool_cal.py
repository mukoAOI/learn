def bool_and(index_list1, index_list2):
    set1=set(index_list1)
    set2=set(index_list2)
    return list(set1&set2)


def bool_or(index_list1, index_list2):
    set1=set(index_list1)
    set2=set(index_list2)
    return list(set1|set2)

def bool_not(index_list1, index_list2):
    set1=set(index_list1)
    set2=set(index_list2)
    return list(set1-set2)

