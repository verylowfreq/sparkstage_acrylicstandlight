# アクスタ用ライト 完成品向けファームウェア

## ビルト方法

Arduino IDEでM5StampS3向けにビルドしてください。
M5StampS3上のボタンの操作なしに書き込めるように、ツールメニューから USB DFU On Boot -> Enabled、USB Mode -> USB-OTG とすることをおすすめします。（初回書き込みはボタン操作必須です）

HTML/CSS/Javascriptは配列としてプログラムに埋め込んでいます。Pythonスクリプト `generate_html_data.py` を実行して `www_data.c` を更新してください。

M5Stack社のDUAL BUTTON UNITをGroveコネクタに接続したときのコード例もあります。ボタン操作で動作モード、色の変更をします。コード冒頭の `#define USE_DUAL_BUTTON_UNIT` を有効にしてください。
