// Beatsmap Const Identifier
// 譜面定数特定機
// Written by : physics0523

// 【重要】譜面定数は100倍した形で使用するので、注意
// Ex) 13.7 -> 1370
// 譜面定数に小数第3位が導入されたら詰む?知らんな...
// さらに、このコードでは譜面定数の小数第2位は存在しないものとする
// 仮にするようになった場合、physics0523が修正します。多分。
// 最悪、空間計算量を O(曲数 ^ 2) くらい食います。曲数が5000とかになるとせぐふぉするかも。

#include<bits/stdc++.h>

using namespace std;

// 十分大きな値として inf を定義
const int inf = 1000000007;

// ConstIneq[ 曲ID ] : pair<int,int> の vector
// .first <= (曲IDの譜面定数) <= .second
vector<pair<int, int>> ConstIneq;

// RelateSong[ 曲ID ] : set<int> の vector
// RelateSong[ i ] の要素 = (曲ID i)と何らかの関係式が存在する曲IDの集合
vector<set<int>> RelateSong;

// ReqMin[ (曲ID : s , 曲ID : t) ] = {数値v} の、
// keyが pair<int,int> であり value が int である map
// 「 (tの譜面定数) >= (sの譜面定数) + v 」(vが負になる可能性もある)
// 最も緩い制約は、v=-inf
map<pair<int, int>, int> ReqMin;

// ReqMax[ (曲ID : s , 曲ID : t) ] = {数値v} の、
// keyが pair<int,int> であり value が int である map
// 「 (tの譜面定数) <= (sの譜面定数) + v 」(vが負になる可能性もある)
// 最も緩い制約は、v=inf
map<pair<int, int>, int> ReqMax;

// 総曲数(曲IDの最大値)を取得
// もし4難易度全てについて調べるなら、収録曲数の4倍必要
int GetSongNum() {
    int res;
    // 標準入力から入力する
    cin >> res;
    return res;
}

// 譜面定数が小数第1位までしか持たないことを前提にして、
// cfir <= (定数) <= csec を検証し、範囲を狭める
// Ex)
// 1328 <= (定数) <= 1342
// -> 1330 <= (定数) <= 1340
// 1333 <= (定数) <= 1339
// -> 矛盾しているので、エラーを送出
// 返り値 : bool
// true - 異常なし , false - 矛盾が発生
bool ConstValidate(int& cfir, int& csec) {
    // 10単位(本来の0.1単位)に丸める
    if (cfir % 10 != 0) { cfir += (10 - cfir % 10); }
    csec -= (csec % 10);

    if (cfir > csec) { return false; }
    return true;
}

// [曲ID] の譜面定数が first 以上 second 以下であるとの情報を ConstIneq に追加
// (ConstValidateを含む)
// 返り値 : bool
// true - 異常なし , false - 矛盾が発生
bool ConstUpdate(int id, int first, int second) {
    int cfir = max(ConstIneq[id].first, first);
    int csec = min(ConstIneq[id].second, second);

    if (!ConstValidate(cfir, csec)) { return false; }

    ConstIneq[id].first = cfir;
    ConstIneq[id].second = csec;
    return true;
}

// 曲IDがid番の曲について、筐体表示レベルで定数を初期化
void InitSongLevel(int id) {
    int fir, sec;
    // 標準入力から入力する
    // ここで、譜面定数は100倍した形で使用することに注意
    // 12の場合、 fir=1200 sec=1260
    // 13+の場合、 fir=1370 sec=1390
    // 無効な曲IDであれば、 fir=-1 sec=-2 など無効なデータで初期化
    cin >> fir >> sec;
    ConstIneq[id].first = fir;
    ConstIneq[id].second = sec;
    return;
}

// スコアを (定数) + x という形に変換、xを返す
// 例 :
// 1007500 : 返答は 200 (+2.00なので)
// 990000 : 返答は 60 (+0.60なので)
// 一応、この関数はAまでの定数に対応しておく
// それ未満は、-infを返答とする

// 定数用基準スコア
// BaseScore[i]であれば丁度BaseOffset[i] 加点される
// 間は線形補間
const int BaseElement = 5;
const vector<int> BaseScore = { 1007500,1000000,975000,925000,900000 };
const vector<int> BaseOffset = { 200,100,0,-300,-500 };

int ScoreToOffset(int score) {
    if (score >= BaseScore[0]) { return BaseOffset[0]; }
    for (int i = 1; i < BaseElement; i++) {
        if (score >= BaseScore[i]) {
            // 小数第3位より下は自動切り捨て
            // 計算順に注意! C++の int(32bit符号付き整数) には収まる
            return BaseOffset[i] + ((BaseOffset[i - 1] - BaseOffset[i]) * (score - BaseScore[i])) / (BaseScore[i - 1] - BaseScore[i]);
        }
    }
    // A未満なので、データ破棄
    return -inf;
}

// 追加した情報を処理
// ここがメインパート! むずかしい!
// 下のInputSetData内で呼ばれる
// 返り値 : bool
// true : 成功 , false : データ破損
bool Run(queue<int>& qu) {
    while (!qu.empty()) {
        int cid = qu.front(); // 着目する曲ID
        qu.pop(); // queueの先頭削除
        for (auto nid : RelateSong[cid]) { // nid を cid と関連する曲ID全てについて走査
            bool change = false; // nidに関する制約に変動があったかどうか
            pair<int, int> CNp = make_pair(cid, nid);
            // 最大値規定制約
            if (ConstIneq[cid].second + ReqMax[CNp] < ConstIneq[nid].second) { change = true; }
            // 最小値規定制約
            if (ConstIneq[cid].first + ReqMin[CNp] > ConstIneq[nid].first) { change = true; }

            // ConstIneq[nid] に変動が生じる場合
            if (change) {
                // 新たな制約を追加して壊れたら、破滅
                if (!ConstUpdate(nid, ConstIneq[cid].first + ReqMin[CNp], ConstIneq[cid].second + ReqMax[CNp])) {
                    return false;
                }
                // nid を基準とする処理を処理待ち queue に追加
                qu.push(nid);
            }
        }
    }
    return true;
}

// Best枠 (Recent枠) 情報入力
// 返り値 : bool
// true : 成功 , false : データ破損
bool InputSetData() {
    // 入力は全て標準入力から行っている

    int SetSong;
    cin >> SetSong; // 何曲からなるデータか? (Bestなら30、Recentなら10)
    vector<int> Songid, Offset; // 初めは空
    for (int i = 0; i < SetSong; i++) {
        int inid, insc;
        cin >> inid >> insc;
        insc = ScoreToOffset(insc); // スコアをオフセットに変換
        if (insc < 0) { continue; } // Sに満たない、オフセット 0 未満ならデータを破棄
        Songid.push_back(inid);
        Offset.push_back(insc);
    }
    SetSong = Songid.size(); // 有効なデータが何曲あるかに更新

    // i-1個目とi個目のデータについて、データを追加
    for (int i = 1; i < SetSong; i++) {
        int Uid = Songid[i - 1], Uofs = Offset[i - 1]; // 順位が高い方
        int Lid = Songid[i], Lofs = Offset[i]; // 順位が低い方
        if (Uid == Lid) { continue; } // 同じ曲同士なら、処理を無視
        pair<int, int> ULp = make_pair(Uid, Lid);
        pair<int, int> LUp = make_pair(Lid, Uid);

        //はじめての関係なら、その関係の初期化処理を行う
        if (RelateSong[Uid].find(Lid) == RelateSong[Uid].end()) {
            //関係を追加
            RelateSong[Uid].insert(Lid);
            RelateSong[Lid].insert(Uid);
            //最も甘い条件で初期化
            ReqMin[ULp] = -inf;
            ReqMax[ULp] = inf;
            ReqMin[LUp] = -inf;
            ReqMax[LUp] = inf;
        }

        // (上の定数) + Uofs >= (下の定数) + Lofs
        // 上を基準に、下の最大値制約が規定、必要があれば更新される
        ReqMax[ULp] = min(Uofs - Lofs, ReqMax[ULp]);
        // 下を基準に、上の最小値制約が規定、必要があれば更新される
        ReqMin[LUp] = max(Lofs - Uofs, ReqMin[LUp]);
        // ここで、minとmaxが逆に対応することに注意(これで正しいです)
    }

    queue<int> qu; // Runに渡すqueue
    for (int i = 0; i < SetSong; i++) { qu.push(Songid[i]); } // 関係が動きうる全曲を追加
    return Run(qu); // 影響を伝播
}

// 単曲制約追加
// 返り値 : bool
// true : 成功 , false : データ破損
bool InputOneData() {
    // 入力は全て標準入力から行っている
    int id, first, second;
    cin >> id >> first >> second;
    if (!ConstUpdate(id, first, second)) { return false; } // 単曲で定数制約更新
    queue<int> qu;
    qu.push(id);
    return Run(qu); // 影響を伝播
}

int main() {
    // 実運用の場合、初期化を行うのは最初の1回だけでよい
    int SongNum = GetSongNum(); // 総曲数(曲IDの最大値)の取得
    ConstIneq.resize(SongNum + 1);
    RelateSong.resize(SongNum + 1);
    ConstIneq[0] = make_pair(-1, -2); // 曲ID0番に、無効なデータを番兵として置く
    for (int i = 1; i <= SongNum; i++) {
        InitSongLevel(i); // 曲IDがi番の曲について、筐体表示レベルを入力
    }

    // Best枠 (Recent枠) 情報入力ゾーン
    // 旧バージョンの状態の各枠を間違って取り込まないよう、注意!
    // (直近プレイ記録の最新プレイ時刻とかで判定してください)
    while (1) {
        // 標準入力から Set/One/No 入力 (Case-sensitive)
        // "Set" ... 枠の情報を入力する
        // "One" ... ある曲の定数の情報を入力する
        // "No" ... 終わる
        string s;
        while (1) {
            cin >> s;
            if (s == "Set" || s == "One" || s == "No") { break; }
            cout << "Error : Input should be Set / One / No (Case-sensitive)\n";
        }
        if (s == "No") { break; }
        // Best枠かRecent枠のデータを、1個分取得する
        if (s == "Set") {
            if (!InputSetData()) {
                // データがどこかで破損している
                cout << "Error : Crashed!!\n";
                return 0;
            }
        }
        // ある曲について、制約を追加
        if (s == "One") {
            if (!InputOneData()) {
                // データがどこかで破損している
                cout << "Error : Crashed!!\n";
                return 0;
            }
        }
    }

    // 終了処理
    for (int i = 1; i <= SongNum; i++) {
        // 最終結果の出力
        cout << i << ":[" << ConstIneq[i].first << "," << ConstIneq[i].second << "]\n";
    }
    return 0;
}

// 以下、テストケース入力形式のメモ
// N
// A_i B_i
// Set
// M
// Id_j Score_j
// One
// Id_k A_k B_k
// ...
// No

// N...総曲数
// A_i <= B_i ... 制約
// M...データをなす曲数
// Id_j ... 曲ID
// Score_j .. スコア
// Id_k A_k B_k ... 単曲制約
