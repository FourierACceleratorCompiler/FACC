type synth_result = {
	function_code: string
	function_name: string
}

type synth_mode =
	| NoSynthesizer
	| InternalFFTSynth
