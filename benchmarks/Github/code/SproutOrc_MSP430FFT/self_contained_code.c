typedef struct {
    float real;
    float imag;
} Complex;

void imagSetToZero(Complex *data, int num) 
{
    for (int i =  0; i < num; i++) {
        (data + i)->imag = 0.0;
    }
}

//bit reversal for resequencing data
//Rader algorithm
//Input reverse
void inReverse(Complex *data, int num) { 
    Complex Temp;   
    int j = 0;
    int k = 0;
    
    for (int i = 1; i < (num - 1); i++) {
        k = num / 2;

        while (k <= j) {
            j = j - k;
            k = k / 2;
        }
        j = j + k;
        if (i < j) {
            Temp.real = (data + j)->real;
            Temp.imag = (data + j)->imag;

            (data + j)->real = (data + i)->real;
            (data + j)->imag = (data + i)->imag;

            (data + i)->real = Temp.real;
            (data + i)->imag = Temp.imag;
        }
    }
}

void addComplex(Complex *getResult, Complex const *A, Complex const *withB) {
    getResult->real = A->real + withB->real;
    getResult->imag = A->imag + withB->imag;
}

void subComplex(Complex *getResult, Complex const *A, Complex const *withB) {
    getResult->real = A->real - withB->real;
    getResult->imag = A->imag - withB->imag;
}

void mulComplex(Complex *getResult, Complex const *A, Complex const *withB) {
    getResult->real = A->real * withB->real - A->imag * withB->imag;
    getResult->imag = A->real * withB->imag + A->imag * withB->real;
}

//input sample array, # of points
void FFT(Complex *data, Complex const *factor, int num, int series)
{
    //temporary storage variables 
    Complex topData;
    Complex lowData;
    Complex dataIndex;
    //difference between top/lower leg
    int leg_diff;
    int lower_leg;
    //index/step through twiddle constant 
    int index = 0;
    //step between values
    int step = 1;
    //difference between upper & lower legs
    leg_diff = num / 2;
    
    imagSetToZero(data, num);
    
    //for N-point FFT
    for (int i = 0; i < series; i++)                   
    {
        index = 0;
        for (int j = 0; j < leg_diff; j++) {
            for (int upper_leg = j; upper_leg < num; upper_leg += (2 * leg_diff)) {
                lower_leg = upper_leg + leg_diff;

                addComplex(&topData, (data + upper_leg), (data + lower_leg));
                subComplex(&lowData, (data + upper_leg), (data + lower_leg));
                dataIndex.real = (factor + index)->real;
                dataIndex.imag = (factor + index)->imag;
                mulComplex((data + lower_leg), &lowData, &dataIndex);
                (data + upper_leg)->real = topData.real;
                (data + upper_leg)->imag = topData.imag;
            }
            index += step;
        }

        leg_diff >>= 1;
        step *= 2;
    }
    inReverse(data, num);
}