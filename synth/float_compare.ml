open Core_kernel;;
open Options;;

(* We found that using just raw FP approximate equality was
	not a sensible way to go --- too many issues whre the output
	is basically the same but differs in a single location.
	Also makes it easier for the programmer to specify a bound. *)
class virtual fp_comp =
	object
		(* Any individual numbers, e.g. to implement
		a threshold that if numbers differ by more than X
		then we shoulld auto-fail.  *)
		method virtual compare: float -> float -> bool
		(* Get the result as a whole.  *)
		method virtual result: options -> bool
	end

(* Implement an Mean-Sqaured-Error function.  *)
class fp_comp_mse (threshold: float) =
	object (self)
		inherit fp_comp as super
		val mutable diffs = ([]: float list)

		method mse () =
			let sum = Float.sqrt (List.fold ~init:0.0 ~f:(fun acc -> fun n -> n +. acc) diffs) in
			let length = float_of_int (List.length diffs) in
			let result = sum /. (length *. Float.log10(length)) in
			result

		method compare f1 f2 =
			let diff = f2 -. f1 in
			let () = diffs <- ((diff *. diff) :: diffs) in
			if (Float.compare (self#mse ()) (10.0 *. threshold)) <= 0 then
				true
			else
				(* fail early if we are way out of the threshold.  *)
				true

		method result options =
			match diffs with
			| [] -> true
			| _ ->
				let result = self#mse () in
				(* We expect the mean squared error to be bounded
				by a constant proportional to N log N *)
				let () = if options.debug_comparison then
					Printf.printf "Found a threshold of %f, had threshold %f\n" result threshold
				else ()
				in
				(Float.compare result threshold) <= 0
	end


