//
//  main.c
//  evenoddStripe
//
//  Created by 李萍萍 on 2021/10/27.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

//取模运算
int getMod(int m, int n) {
    if(m >= 0) {
        return m % n;
    }else {
        return (m + n) % n;
    }
}

//求S因子：编码
uint8_t computeS(uint8_t **codeArray, int p) {
    uint8_t S = 0;
    for(int i = 1; i < p; i++) {
        S ^= codeArray[p - 1 - i][i];
    }
    return S;
}

//求因子S:解码｜失效两列不为校验列时  Case 4
uint8_t computeSDecodeCase4(uint8_t **codeArray, int p) {
    uint8_t S = 0;
    for(int i = 0; i < p - 1; i++) {
        S ^= codeArray[i][p];
        S ^= codeArray[i][p + 1];
    }
    return S;
}

//求因子S:解码｜失效的两列为 i < m, j = m  Case2
uint8_t computeSDecodeCase2(uint8_t **codeArray, int p, int *errorCol) {
    int krow = getMod(errorCol[0] - 1, p);
    uint8_t S = 0;
    if(krow != p - 1) {
        S ^= codeArray[krow][p + 1];
    }
    for(int i = 0; i < p; i++) {
        krow = getMod(errorCol[0] - i - 1, p);
        if(krow != p - 1) {
            S ^= codeArray[krow][i];
        }
    }
    return S;
}

//编码
void encoding(uint8_t **codeArray, int p) {
    //行校验
    for(int i = 0; i < p - 1; i++) {
        for(int j = 0; j < p; j++) {
            codeArray[i][p] ^= codeArray[i][j];
        }
    }
    //斜校验
    //因子S
    uint8_t S = computeS(codeArray, p);
    //printf("S = %d\n", S);
    int krow;
    for(int i = 0; i < p - 1; i++) {
        codeArray[i][p + 1] ^= S;
        for(int j = 0; j < p; j++) {
            krow = getMod(i - j, p);
            if(krow != p - 1) {
                //printf("codeArray[%d][%d] = %d\t", krow, j, codeArray[krow][j]);
                codeArray[i][p + 1] ^= codeArray[krow][j];
            }
        }
        //printf("\n codeArray[%d][%d] = %d", i, p + 1, codeArray[i][p + 1]);
        //printf("\n*************\n");
    }
}

//编码：stripe
void encodingOnStripe(uint8_t ***codeArrayStripe, int stripeNum, int p) {
    for(int i = 0; i < stripeNum; i++) {
        encoding(codeArrayStripe[i], p);
    }
}

//解码 case1: i = m, j = m + 1
void decodingCase1(uint8_t **codeArray, int p, int *errorCol) {
    //用编码方法可解
    encoding(codeArray, p);
}

//解码 case2: i < m, j = m
void decodingCase2(uint8_t **codeArray, int p, int *errorCol) {
    uint8_t S = computeSDecodeCase2(codeArray, p, errorCol);
    int krow = errorCol[0], temp;
    //解 i 列
    for(int i = 0; i < p - 1; i++) {
        codeArray[i][errorCol[0]] ^= S;
        krow = getMod(krow, p);
        if(krow != p - 1) {
            codeArray[i][errorCol[0]] ^= codeArray[krow][p + 1];
        }
        for(int j = 0; j < p; j++) {
            if(j != errorCol[0]) {
                temp = getMod(i + errorCol[0] - j, p);
                if(temp != p - 1) {
                    //printf("codeArray[%d][%d] = %d\t", temp, j, codeArray[temp][j]);
                    codeArray[i][errorCol[0]] ^= codeArray[temp][j];
                }
            }
        }
        krow++;
    }
    
    //第 j 列 ： 行校验可解
    for(int i = 0; i < p - 1; i++) {
        for(int j = 0; j < p; j++) {
            codeArray[i][errorCol[1]] ^= codeArray[i][j];
        }
    }
}

//解码 case3: i < m, j = m + 1
void decodingCase3(uint8_t **codeArray, int p, int *errorCol) {
    //用行校验解 i 列
    for(int i = 0; i < p - 1; i++) {
        for(int j = 0; j <= p; j++) {
            if(j != errorCol[0]) {
                codeArray[i][errorCol[0]] ^= codeArray[i][j];
            }
        }
    }
    //用编码方法解 j 列
    uint8_t S = computeS(codeArray, p);
    int krow;
    for(int i = 0; i < p - 1; i++) {
        codeArray[i][errorCol[1]] ^= S;
        for(int j = 0; j < p; j++) {
            krow = getMod(i - j, p);
            if(krow != p - 1) {
            codeArray[i][errorCol[1]] ^= codeArray[krow][j];
            }
        }
    }
}

//求S0 解码case4
uint8_t getS0(uint8_t **codeArray, int p, int row, int *errorCol) {
    uint8_t S0 = 0;
    if(row != p - 1) {
       for(int j = 0; j <= p; j++) {
            if(j != errorCol[0] && j != errorCol[1]) {
                S0 ^= codeArray[row][j];
            }
        }
    }
    return S0;
}

//求S1 解码case4
uint8_t getS1(uint8_t **codeArray, int p, int row, int *errorCol) {
    uint8_t S = computeSDecodeCase4(codeArray, p);
    uint8_t S1 = 0;
    S1 ^= S;
    int krow;
    if(row != p - 1) {
        S1 ^= codeArray[row][p + 1];
    }
    
    for(int i = 0; i < p; i++) {
        if(i != errorCol[0] && i != errorCol[1]) {
            krow = getMod(row - i, p);
            if(krow != p - 1) {
                S1 ^= codeArray[krow][i];
            }
        }
    }
    return S1;
}

//解码 case4: i < m, j < m
void decodingCase4(uint8_t **codeArray, int p, int *errorCol) {
    int k = getMod(-(errorCol[1] - errorCol[0]) -1, p);
    int krow;
    uint8_t S0, S1;
    while(k != p - 1) {
        // as,j = s1<j + s>m + a<s + (j - i)>m,i
        krow = getMod(errorCol[1] + k, p);
        S1 = getS1(codeArray, p, krow, errorCol);
        codeArray[k][errorCol[1]] ^= S1;
        krow = getMod(k + (errorCol[1] - errorCol[0]), p);
        if(krow != p - 1) {
            codeArray[k][errorCol[1]] ^= codeArray[krow][errorCol[0]];
        }
        S0 = getS0(codeArray, p, k, errorCol);
        codeArray[k][errorCol[0]] ^= S0;
        codeArray[k][errorCol[0]] ^= codeArray[k][errorCol[1]];
        
        k = getMod(k - (errorCol[1] - errorCol[0]), p);
    }
}

//解码
void decoding(uint8_t **codeArray, int p, int *errorCol) {
    if(errorCol[0] == p && errorCol[1] == p + 1) {
        decodingCase1(codeArray, p, errorCol);
    }else if(errorCol[0] < p && errorCol[1] == p) {
        decodingCase2(codeArray, p, errorCol);
    }else if(errorCol[0] < p && errorCol[1] == p + 1) {
        decodingCase3(codeArray, p, errorCol);
    }else if (errorCol[0] < p && errorCol[1] < p) {
        decodingCase4(codeArray, p, errorCol);
    }
}

//解码：stripe
void decodingOnStripe(uint8_t ***codeArrayStripe, int stripeNum, int p, int *errorCol) {
    for(int i = 0; i < stripeNum; i++) {
        decoding(codeArrayStripe[i], p, errorCol);
    }
}

//随机赋值
void randValue(uint8_t **codeArray, int p) {
    srand((unsigned)time(NULL));
    for(int i = 0; i < p - 1; i++) {
        for(int j = 0; j < p; j++) {
            codeArray[i][j] = rand();
        }
        codeArray[i][p] = 0;
        codeArray[i][p + 1] = 0;
    }
}

//随机赋值：stripe
void randValueOnStripe(uint8_t ***codeArrayStripe, int stripeNum, int p) {
    for(int i = 0; i < stripeNum; i++) {
        randValue(codeArrayStripe[i], p);
    }
}

//判断素数
int isPrime(int num) {
    for (int i = 2; i * i <= num; i++) {
        if(num % i == 0) {
            return 0;
        }
    }
    return 1;
}

//保存原始数据
void saveOrignalValue(uint8_t **codeArray, uint8_t **orignalArray, int p) {
    for(int i = 0; i < p - 1; i++) {
        for(int j = 0; j <= p + 1; j++) {
            orignalArray[i][j] = codeArray[i][j];
        }
    }
}

//保存原始数据：stripe
void saveOrignalValueOnStripe(uint8_t ***codeArrayStripe, uint8_t ***orignalStripe, int stripeNum, int p) {
    for(int i = 0; i < stripeNum; i++) {
        saveOrignalValue(codeArrayStripe[i], orignalStripe[i], p);
    }
}

//置两列失效
void setTwoError(uint8_t **codeArray, int p, int *errorCol) {
    for(int i = 0; i < p - 1; i++) {
        codeArray[i][errorCol[0]] = 0;
        codeArray[i][errorCol[1]] = 0;
    }
}

//置两列失败：stripe
void setTwoErrorOnStripe(uint8_t ***codeArrayStripe, int stripeNum, int p, int *errorCol) {
    for(int i = 0; i < stripeNum; i++) {
        setTwoError(codeArrayStripe[i], p, errorCol);
    }
}

//判断恢复是否正确
int isEqual(uint8_t **codeArray, uint8_t **orignalArray, int p) {
    for(int i = 0; i < p - 1; i++) {
        for(int j = 0; j <= p + 1; j++) {
            if(codeArray[i][j] != orignalArray[i][j]) {
                return 1;
            }
        }
    }
    return 0;
}

//判断恢复是否正确：stripe
int isEqualOnStripe(uint8_t ***codeArrayStripe, uint8_t ***orignalStripe, int stripeNum, int p) {
    int isSuccess;
    for(int i = 0; i < stripeNum; i++) {
        isSuccess = isEqual(codeArrayStripe[i], orignalStripe[i], p);
        if(isSuccess) {
            return 1;
        }
    }
    return 0;
}

//输出
void printArray(uint8_t **codeArray, int p) {
    for(int i = 0; i < p - 1; i++) {
        for(int j = 0; j <= p + 1; j++) {
            printf("%d\t", codeArray[i][j]);
        }
        printf("\n");
    }
}

//输出：stripe
void printArrayOnStripe(uint8_t ***codeArrayStrip, int stripeNum, int p) {
    for(int i = 0; i < stripeNum; i++) {
        printArray(codeArrayStrip[i], p);
    }
}

int main(int argc, const char * argv[]) {
    // insert code here...
    //条带规模
    int p = 337, stripeNum = 100;
    //错误列
    int errorCol[2] = {24, 33};
    //时间
    struct timeval encodeStart, encodeEnd, decodeStart, decodeEnd;
    long encodeTime, decodeTime;
    //容量
    long long memory = sizeof(uint8_t) * 8 * (p - 1) * p * stripeNum;
    //是否解码成功
    int isSuccess;
    uint8_t ***codeArrayStripe = (uint8_t ***) malloc(stripeNum * sizeof(uint8_t **));
    //保存原始数据
    uint8_t ***orignalStripe = (uint8_t ***) malloc(stripeNum * sizeof(uint8_t **));
    //开辟空间
    for(int i = 0; i < stripeNum; i++) {
        codeArrayStripe[i] = (uint8_t **) malloc((p - 1) * sizeof(uint8_t *));
        orignalStripe[i] = (uint8_t **) malloc((p - 1) * sizeof(uint8_t *));
        for(int j = 0; j < p - 1; j++) {
            codeArrayStripe[i][j] = (uint8_t *) malloc((p + 2) * sizeof(uint8_t));
            orignalStripe[i][j] = (uint8_t *) malloc((p + 2) * sizeof(uint8_t));
        }
    }
    
    //随机赋值
    randValueOnStripe((uint8_t ***) codeArrayStripe, stripeNum, p);
    
    //编码
    gettimeofday(&encodeStart, NULL);
    encodingOnStripe((uint8_t ***) codeArrayStripe, stripeNum, p);
    gettimeofday(&encodeEnd, NULL);
    
    //编码时间
    encodeTime = (encodeEnd.tv_sec - encodeStart.tv_sec) * 1000000 + encodeEnd.tv_usec - encodeStart.tv_usec;
    
    //保存数据
    saveOrignalValueOnStripe((uint8_t ***) codeArrayStripe, (uint8_t ***) orignalStripe, stripeNum, p);
    
    //置0
    setTwoErrorOnStripe((uint8_t ***) codeArrayStripe, stripeNum, p, errorCol);
    
    //解码
    gettimeofday(&decodeStart, NULL);
    decodingOnStripe((uint8_t ***) codeArrayStripe, stripeNum, p, errorCol);
    gettimeofday(&decodeEnd, NULL);
    
    //解码时间
    decodeTime = (decodeEnd.tv_sec - decodeStart.tv_sec) * 1000000 + decodeEnd.tv_usec - decodeStart.tv_usec;
    
    //是否解码成功
    isSuccess = isEqualOnStripe((uint8_t ***) codeArrayStripe, (uint8_t ***) orignalStripe, stripeNum, p);
    printf("p = %d, stripeNum = %d\n", p, stripeNum);
    printf("失效列： %d, %d\n", errorCol[0], errorCol[1]);
    printf("编码速度：%.2f Gb/s \n", (double)memory / encodeTime / 1024.0 / 1024.0 / 1024.0 * 1000000);
    if(!isSuccess) {
        printf("解码成功！\n");
        printf("解码速度：%.2f Gb/s \n", (double)memory / decodeTime / 1024.0 / 1024.0 / 1024.0 * 1000000);
    } else {
        printf("解码失败！\n");
    }
    return 0;
}
