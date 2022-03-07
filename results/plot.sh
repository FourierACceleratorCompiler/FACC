# Biggest --- require a log axis.
python3.9 plot_joint_graph.py all  Github/mborgerding_kissfft/Github/gregfjohnson_fft Github/cpuimage_cpuFFT/ Github/xbarin02_uFFT_2/ Github/MiBench_MiBench Github/liscio_fft Github/jtfell_c-fft

# Next biggest, up to about 60x speedup.
python3.9 plot_joint_graph.py slow Github/JodiTheTigger_meow_fft/ Github/marton78_pffft/ Github/omnister_fftp Github/dlinyj_fft/ Github/xbarin02_uFFT/ Github/cpuimage_FFTResampler/  Github/akw0088_fft/

python3.9 plot_joint_graph.py faster Github/mozanunal_SimpleDSP/ Github/tasimon_FFT/ Github/cpuimage_StockhamFFT/ Github/xiahouzouxin_fft/

# python plot_joint_graph.py fast_only Github/cpuimage_cpuFFT Github/cpuimage_FFTResampler Github/cpuimage_StockhamFFT Github/dlinyj_fft Github/JodiTheTigger_meow_fft Github/marton78_pffft Github/mborgerding_kissfft Github/mozanunal_SimpleDSP Github/tasimon_FFT Github/xbarin02_uFFT Github/xbarin02_uFFT_2 Github/xiahouzouxin_fft

# Plot the bar chart idl/programl/
python3.9 plot_success_comps.py CompileStatistics.dat

python3.9 plot_idl_graph.py ../benchmarks/Github/code/FFT_IDL_Check "Pattern 0"
