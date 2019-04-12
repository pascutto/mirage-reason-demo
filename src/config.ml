open Mirage

(* Network configuration *)

let stack = generic_stackv4 default_network

let data = generic_kv_ro "../site"
(* Dependencies *)

let server =
  foreign "Unikernel.Make"
    (console @-> pclock @-> kv_ro @-> conduit @-> http @-> job)

let conduit = (conduit_direct stack)

let app =
  httpaf_server conduit

let () =
  register "httpaf_unikernel"
  ~packages:[package "mirage-websocket"; package "mirage-http"]
  [ server $ default_console $ default_posix_clock$ data $ conduit $ app ]