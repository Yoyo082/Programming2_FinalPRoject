#不要在preview看，格式會是錯的
## Git 工作流程指南

一次性設定（第一次才需要做）

設定名字和信箱：
git config --global user.name "你的名字"
git config --global user.email "你的信箱"

Clone 專案到本機：
git clone https://github.com/Yoyo082/Programming2_FinalPRoject.git
cd Programming2_FinalPRoject


---
每次工作的流程

git pull origin main

寫完程式後，上傳到 GitHub：
git status                  # 確認哪些檔案有變動
git add .                   # 加入所有變動
git commit -m "做了什麼"    # 儲存版本
git push origin main        # 推上 GitHub

常見問題

Push 被拒絕
代表有人比你先 push，執行以下指令再重新 push：

git pull origin main
git push origin main

**跳出 Vim 黑色畫面**
直接輸入 :wq 再按 Enter 關掉即可。


e.g.
    git commit -m "[State] 實作夜晚行動執行順序"

[State] 狀態機、日夜循環、主迴圈
[Win] 勝負判定邏輯
[UI] Raylib 繪圖、點擊事件
[Vote] 投票、PK系統
[Role] 角色資料結構、技能邏輯
[Audio] 音效、旁白語音
[Log] 文字記錄、歷史log
[Fix] 修正bug
[Docs] 更新文件或 README

寫的時候記得 .h 和 .c 要分開（標頭檔和實作檔）