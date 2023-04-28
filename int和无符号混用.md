# <center>int和无符号混用
在使用for循环的时候，出现了一个问题，代码如下：
```c++
// st.size() 为 0 ，k = 9
for (i = 1 ; i <= 9 - (k - st.size() + 1); i++) {
			st.push_back(i);
			//cout << i << " "<< st.size() << "|" << endl;
			surplus -= i;
			/*if (surplus < 0) {
				st.pop_back();
				surplus += i;
				return;
			}*/
			backtracking(k, surplus, i + 1);
			st.pop_back();
			surplus += i;
		}
```
计算出来 1 < -1 循环引应该跳出来才对，但是惊奇的进去了，我看了好久好久好久，终于看到了原因：
s.size()函数返回值的实际类型是string::size_type，该类型是一个**无符号整型数**。在表达式中混用unsigned int与int可能产生意想不到的结果。
-  也就是说 其实他的运算结果并非我们想象的那样，但是我还是有些疑惑的是，为什么vs给出的计算结果是按我想象中的给的？
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230220165347783-1327951971.png)
这是我很疑惑的点，所以我做了测试：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230220170924838-25588138.png)![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230220172139049-1624172979.png)
但是根据上面的解释，我调试的时候并没有注意到判断的类型是unsigned类型,所以出现了这样的错误。
我进行强制转化，就没出现上述不能理解的，莫名奇妙的进入循环的问题，如下：
```c++
// st.size() 为 0 ，k = 9
for (i = 1 ; i <= 9 - (k - (int)st.size() + 1); i++) {
			st.push_back(i);
			//cout << i << " "<< st.size() << "|" << endl;
			surplus -= i;
			/*if (surplus < 0) {
				st.pop_back();
				surplus += i;
				return;
			}*/
			backtracking(k, surplus, i + 1);
			st.pop_back();
			surplus += i;
		}

```

