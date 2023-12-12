# -*- coding: utf-8 -*-
from git import Repo
import tkinter
import tkinter.messagebox
 
# 有未提交更改时提示
repo = Repo('.')
verstr = repo.git.log('-1','--pretty=format:%cd hash:%h')
if repo.is_dirty():
    a=tkinter.messagebox.askokcancel('提示', '有未提交修改, 确定更新版本信息?')
    if not a:
        print("取消更新")
        raise
    verstr += ' modified'
 
fi = open('./GitVerison.h', 'w', encoding='utf-8')
fstr = '#ifndef _GITVER_H\n#define _GITVER_H\n#define GIT_VER "' + verstr + '"\n#endif'
fi.write(fstr)
fi.close()
 
print('成功更新git信息')
print(verstr)