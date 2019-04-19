# RunawayGem 暴走宝石

璀璨宝石游戏的简单 AI。
A simple AI for game Splendor.

## 程序运行
对于输入的当前局面进行处理，计算得出玩家当前要做的操作， 并输出。即，运行一次程序只有一次输入和一次输出，只决策当前的一个操作。输入为传给 argv[1] 一个 json 文件路径，输出到标准输出(stdin 和 stdout)。

## 输入/输出数据格式
所有的输入、输出数据都采用 JSON 格式，使用了 JSONcpp 库。

## AI 对战


解压 Splendor.tar.bz2.\* 得到 Splendor.jar

```shell
cat Splendor.tar.bz2.* | tar -jxv
```

对战

```shell
java -jar Splendor.jar --player_binary_paths player1,player2,player3 --game_log_path log.txt
```

player1,player2,player3 是编译好的二进制, 注意必须名字不一样。
