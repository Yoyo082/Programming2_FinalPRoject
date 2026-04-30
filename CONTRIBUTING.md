git and github


每個人在終端機輸入：
git clone https://github.com/Yoyo082/Programming2_FinalPRoject.git
cd Programming2_FinalPRoject
code .

每次開始前在終端機輸入：
git checkout dev
git pull origin dev

建立自己的功能分支：
git checkout -b feature/你的功能名稱

寫完一個段落後，儲存進度：
git add .
git commit -m "簡短描述做了什麼"
git push origin feature/你的功能名稱

e.g.
	git commit -m "[State] 實作夜晚行動執行順序"

[State] 狀態機、日夜循環、主迴圈
[Win] 勝負判定邏輯
[UI]Raylib 繪圖、點擊事件
[Vote]投票、PK系統
[Role]角色資料結構、技能邏輯
[Audio]音效、旁白語音
[Log]文字記錄、歷史log
[Fix]修正bug
[Docs]更新文件或 README

然後寫的時候記得.h 和 .c 要分開 (標投檔和實作檔)











