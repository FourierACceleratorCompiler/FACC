import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

RESULTS_PATH = 'results3.csv'


def main():
    df = pd.read_csv(RESULTS_PATH)
    for key in df:
        print(key)
        df[key] = df[key].dropna()
    sns.lineplot(x=df['instances_per_class'], y=df['Test_accuracy_top'], marker=r'o', ci='sd', color='red',
                 label='Overall top-3 accuracy')

    sns.lineplot(x=df['instances_per_class'], y=df['Test_accuracy'], marker=r'o', ci='sd', linestyle='--', color='red',
                 label='Overall top-1 accuracy')
    # sns.lineplot(x=df['instances_per_class'], y=df['Validation_FFT_accuracy'], marker=r'o', ci='sd',
    #              label='FFT top-1 accuracy')
    #sns.lineplot(x=df['instances_per_class'], y=df['Test_recall_ffts'], marker=r'o', ci='sd', color='#5060d0', linestyle='--',
    #             label='FFT recall')
    #sns.lineplot(x=df['instances_per_class'], y=df['Test_precision_ffts'], marker=r'o', ci='sd', color='orange',
    #             label='FFT precision')
    #sns.lineplot(x=df['instances_per_class'], y=df['Test_f1_ffts'], marker=r'o', ci='sd', color='green',
    #             label='FFT F1')
    sns.lineplot(x=df['instances_per_class'], y=df['Test_recall_ffts_top'], marker=r'o', ci='sd', color='#5060d0',
                 label='FFT top-3 recall')
    plt.xlabel('Train instances per class')
    plt.ylabel('Cross-validation score')
    plt.legend()
    plt.xlim(left=0, right=16)
    plt.ylim(top=1.0, bottom=0.0)
    plt.savefig('plot3.pdf')
    plt.show()


    '''
    sns.lineplot(x=df['instances_per_class'], y=df['Test_TOP_accuracy'], marker=r'o', ci='sd', color='blue',
                 label='Overall top-3 accuracy')

    sns.lineplot(x=df['instances_per_class'], y=df['Test_accuracy'], marker=r'o', ci='sd', linestyle='--', color='blue',
                 label='Overall top-1 accuracy')
    # sns.lineplot(x=df['instances_per_class'], y=df['Validation_FFT_accuracy'], marker=r'o', ci='sd',
    #             label='FFT top-1 accuracy')
    sns.lineplot(x=df['instances_per_class'], y=df['Test_TOP_FFT_accuracy'], marker=r'o', ci='sd', color='red',
                 label='FFT top-3 accuracy')
    plt.xlabel('Train instances per class')
    plt.ylabel('Cross-validation accuracy')
    plt.legend()
    plt.xlim(left=0, right=16)
    plt.ylim(top=1.0, bottom=0.0)
    plt.savefig('plot.pdf')
    plt.show()
    '''


if __name__ == '__main__':
    main()
