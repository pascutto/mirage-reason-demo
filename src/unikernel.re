open Lwt.Infix;
open Httpaf;

module type HTTP = Httpaf_mirage.Server_intf;

module Dispatch = (C: Mirage_types_lwt.CONSOLE, FS : Mirage_types_lwt.KV_RO, Http: HTTP) => {

  let log = (c, fmt) => Printf.ksprintf (C.log(c), fmt);

  let rec get_content = (fs, c, path) =>
    switch path {
    | "/hello" => get_content(fs, c, "/index.html")
    | "/goodbye" => get_content(fs, c, "/goodbye.html")
    | "/" => get_content(fs, c, "/index.html")
    | _   =>
      FS.get(fs, Mirage_kv.Key.v (path)) >>= (res) => switch res {
        | Error (_) => failwith ("erreur")
        | Ok (body) => Lwt.return (body)
      }
    };

  let dispatcher = (fs, c, reqd) => {
    let req = Reqd.request(reqd);
    let target = req.target;
    Lwt.catch
      (() =>
         get_content (fs, c, target) >|= (body) => {
         let response = Response.create(
            ~headers=(Headers.of_list ([("Content-Length", body
                |> String.length
                |> string_of_int)])),
            `OK);
         Reqd.respond_with_string (reqd, response, body)},
      ((exn) => {
        let response = Response.create (`Internal_server_error);
        Lwt.return (Reqd.respond_with_string (reqd, response, (Printexc.to_string (exn))))}
      ))
    |> ignore
  };

  let serve = (c, dispatch) => {
    let error_handler = (~request=?, _error, mk_response) => {
      let response_body = mk_response (Headers.empty);
      Body.write_string (response_body, "Error handled");
      Body.flush (response_body, (() => Body.close_writer (response_body)))
    };
    Http.create_connection_handler(
      ~config=?None,
      ~request_handler=(dispatch (c)),
      ~error_handler)
  };

};

/* Server boilerplate */
module Make = (C : Mirage_types_lwt.CONSOLE, Clock : Mirage_types_lwt.PCLOCK, FS : Mirage_types_lwt.KV_RO, Http: HTTP) => {

  module D  = Dispatch (C, FS, Http);

  let log = (c, fmt) => Printf.ksprintf (C.log(c), fmt);

  let start = (c, _clock, fs, http) =>
    log (c, "started unikernel listen on port 8001") >>= () =>
    http ((`TCP (8001)), (D.serve (c, D.dispatcher (fs))));

  };
