<font face = "Consolas" color = "Blue" size = 3> activation test </font>
```C++
    LweSample *A;
    bit4_activation AC;
    A = new_gate_bootstrapping_ciphertext_array(9, global_params);
    // little-endian
    bootsSymEncrypt(&A[0], 0, global_key);
    bootsSymEncrypt(&A[1], 1, global_key);
    bootsSymEncrypt(&A[2], 1, global_key);
    bootsSymEncrypt(&A[3], 0, global_key);
    bootsSymEncrypt(&A[4], 0, global_key);
    bootsSymEncrypt(&A[5], 1, global_key);
    bootsSymEncrypt(&A[6], 1, global_key);
    bootsSymEncrypt(&A[7], 1, global_key);

    bootsSymEncrypt(&A[8], 1, global_key);

    cout << "[BEFORE INFO]:" << endl;
    for(int i = 0; i< 9; i++) {
        cout << bootsSymDecrypt(&A[i], global_key);
    }
    cout << endl;

    AC._relu(A);
    AC._show_result();
```

<font face = "Consolas" color = "Blue" size = 3> multiply operation test </font>
```C++
    bit4_multiplier M(minimum_lambda, seed);
    int32_t messageA[4] = {0};
    int32_t messageB[4] = {0};
    
    int32_t _iter = 1000;

    for(int i = 0; i< _iter; i++) {
        R._get_rand_seq(messageA, messageB, 4);
        double start_time = clock();
        M._multiply_evaluate(messageA, messageB);
        double end_time = clock();
        R._export_time_to_excel(i, _iter, messageA, messageB, end_time - start_time);
    }

    Booth implmentation.
    double Booth_start_time = clock();
    M._Booth_multiply_evaluate(messageA, messageB);
    double Booth_end_time = clock();
    cout << (Booth_end_time - Booth_start_time)/CLOCKS_PER_SEC << "s" << endl;
    cout << (Booth_end_time - start_time)/CLOCKS_PER_SEC << "s" << endl;
```


<font face = "Consolas" color = "Blue" size = 3> convolution operation test </font>
```C++
    LweSample **** I;
    bit4_convolution C(3, 0, 2, 2, 2, 1);
    C._set_default_values();

    cout << "default values is completed." << endl;

    I = new LweSample***[2];
    for(int ic = 0 ; ic < 2; ic++) {
        I[ic] = new LweSample**[3];
        for(int c = 0; c < 3 ; c++) {
            I[ic][c] = new LweSample*[3];
            for(int r = 0; r < 3; r++) {
                I[ic][c][r] = new_gate_bootstrapping_ciphertext_array(9, global_params);
                bootsSymEncrypt(&I[ic][c][r][0], 1, global_key);
                bootsSymEncrypt(&I[ic][c][r][1], 1, global_key);
                for(int i = 2; i < 9; i++) {
                    bootsSymEncrypt(&I[ic][c][r][i], 0, global_key);
                }
            }
        }
    }
    cout << "input layer is loaded." << endl;
    C._conv_evaluate(I);
    C._show_result();
```

<font face = "Consolas" color = "Blue" size = 3> max_pool evaluation test </font>
```C++
    bit4_pool P(2, 3, 2, 1, 0);

    I = new LweSample***[2];
    for(int ic = 0 ; ic < 2; ic++) {
        I[ic] = new LweSample**[3];
        for(int c = 0; c < 2 ; c++) {
            I[ic][c] = new LweSample*[3];
            for(int r = 0; r < 3; r++) {
                I[ic][c][r] = new_gate_bootstrapping_ciphertext_array(9, global_params);
                bootsSymEncrypt(&I[ic][c][r][0], 1, global_key);
                bootsSymEncrypt(&I[ic][c][r][1], 1, global_key);
                for(int i = 2; i < 9; i++) {
                    bootsSymEncrypt(&I[ic][c][r][i], 0, global_key);
                }
            }
        }
        I[ic][2] = new LweSample*[3];
        for(int r = 0; r < 3; r++) {
            I[ic][2][r] = new_gate_bootstrapping_ciphertext_array(9, global_params);
            bootsSymEncrypt(&I[ic][2][r][0], 1, global_key);
            bootsSymEncrypt(&I[ic][2][r][1], 1, global_key);
            bootsSymEncrypt(&I[ic][2][r][2], 1, global_key);
            for(int i = 3; i < 9; i++) {
                bootsSymEncrypt(&I[ic][2][r][i], 0, global_key);
            }
        }
    }

    P._max_pool(I);
    P._show_result();
```

<font face = "Consolas" color = "Blue" size = 3> linear evaluation test </font>
```C++
    LweSample ** I;
    int** weight;
    bit8_linear L(5,2);

    I = new LweSample*[5];
    for(int i =0; i< 5; i++) {
        I[i] = new_gate_bootstrapping_ciphertext_array(9, global_params);
        bootsSymEncrypt(&I[i][0], 1, global_key);
        for(int j=1; j<9; j++){
            bootsSymEncrypt(&I[i][j], 0, global_key);
        }
    }

    weight = new int*[5];
    for(int i =0; i< 5; i++) {
        weight[i] = new int[2];
        for(int j = 0; j<2; j++) {
            weight[i][j] = 2;
        }
    }

    L._set_weight(weight);
    L._linear_evaluation(I);
    L._show_result();
```

<font face = "Consolas" color = "Blue" size = 3> conv-linear evaluation test </font>
```C++
    bit8_conv_linear CL(2,3,3,3);

    LweSample ****I = new LweSample***[2];
    for(int i =0; i< 2; i++) {
        I[i] = new LweSample**[3];
        for(int j =0; j< 3; j++) {
            I[i][j] = new LweSample*[3];
            for(int k =0; k< 3; k++) { 
                I[i][j][k] = new_gate_bootstrapping_ciphertext_array(9, global_params);
                bootsSymEncrypt(&I[i][j][k][0], 1, global_key);
                for(int p = 1; p < 9; p++) {
                    bootsSymEncrypt(&I[i][j][k][p], 0, global_key);
                }
            }
        }
    }

    int ****weight;
    weight = new int***[3];
    for(int i = 0;i < 3; i++) {
        weight[i] = new int**[2];
        for(int j = 0;j < 2; j++) {
            weight[i][j] = new int*[3];
            for(int k = 0;k < 3; k++) {
                weight[i][j][k] = new int[3];
                for(int l=0; l<3 ;l++) {
                    weight[i][j][k][l] = 1;
                }
            }
        }
    }

    CL._set_weight(weight);
    CL._conv_linear_evaluation(I);
    CL._show_result();
```

<font face = "Consolas" color = "Blue" size = 3> scalable-multiply evaluation </font>
```C++
    randseq R;
    
    int A[16] = {0};
    int W[16] = {0};

    int bitwidth = 16;

    scalable_multiply bit8_mul(bitwidth);

    int _iter = 50;

    for(int i = 0; i< _iter; i++) {
        R._get_rand_seq(A, W, bitwidth);
        double start_time = clock();
        bit8_mul._multiply_evaluate(A,W);
        //bit4_bootsADD(result, 4, A, B, &global_key->cloud);
        double end_time = clock();
        R._export_time_to_excel(i, _iter, A, W, bitwidth, end_time - start_time);
    }
    ```