open Mirage

(* Network configuration *)

let stack = generic_stackv4 default_network

let data = generic_kv_ro "../site"
(* Dependencies *)

let server =
  foreign "Unikernel.Make"
    (console @-> pclock @-> kv_ro @-> http @-> job)

let app =
  httpaf_server (conduit_direct stack)

let () =
  register "httpaf_unikernel"
  [ server $ default_console $ default_posix_clock$ data $ app ]