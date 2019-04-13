open Mirage

(* Network configuration *)

let stack = generic_stackv4 default_network

let data = generic_kv_ro "public"
(* Dependencies *)

let server =
  foreign "Unikernel.Make"
    (console @-> pclock @-> kv_ro @-> conduit @-> http @-> job)

let conduit = (conduit_direct stack)

let app =
  httpaf_server conduit

let () =
  register "httpaf_unikernel"
  ~packages:[
    package ~pin:"git+https://github.com/TheLortex/mirage-websocket.git" "mirage-websocket";
    package ~ocamlfind:["mirage-http.cohttp"] ~pin:"git+https://github.com/mirage/mirage-http.git#wip" "mirage-http";
    package ~pin:"git+https://github.com/anmonteiro/httpaf.git#mirage" "httpaf";
    package ~pin:"git+https://github.com/anmonteiro/httpaf.git#mirage" "httpaf-mirage";
    package ~pin:"git+https://github.com/anmonteiro/httpaf.git#mirage" "httpaf-lwt"]
  [ server $ default_console $ default_posix_clock$ data $ conduit $ app ]