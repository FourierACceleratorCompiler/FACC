open Core_kernel;;
open Spec_definition;;
open Spec_utils;;
open Gir;;
open Gir_utils;;

exception FixTypemapException of string

let rec gir_fix_typemap options (typemap: typemap) gir =
    match gir with
    | Definition(n, escapes, defn_type) ->
            if Hashtbl.mem typemap.variable_map (gir_name_to_string n) then
                ()
            else
				let typ = match defn_type with
				| None -> raise (FixTypemapException "Can't fix typemap with None as a type entry")
				| Some(x) -> x
				in
                let _ = Hashtbl.add typemap.variable_map (gir_name_to_string n) typ in
                let _ = match typemap.original_typemap with
                | Some(t) ->
                        let _ = Hashtbl.add t.variable_map (gir_name_to_string n) typ in
                        ()
                | None ->
                        ()
                in
                ()
	| FunctionDef(name, args, body, vmap) ->
			(* Note that we don't support this right now -- we'd
			need to rebuild the whole function structure. *)
			()
    | IfCond(cond, iftrue, iffalse) ->
            let () = gir_fix_typemap options typemap iftrue in
            let () = gir_fix_typemap options typemap iffalse in
            ()
    | Sequence(seq) ->
            let _ = List.map seq (gir_fix_typemap options typemap) in
            ()
    | Assignment(_, _) -> ()
    | LoopOver(body, indv, maxv) ->
            let () = gir_fix_typemap options typemap body in
            ()
    | Expression(_) -> ()
    | Free(_) -> ()
    | Return(_) -> ()
    | EmptyGIR -> ()
