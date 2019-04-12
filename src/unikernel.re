open Lwt.Infix;
open Httpaf;

module type HTTP = Httpaf_mirage.Server_intf;

module Dispatch = (C: Mirage_types_lwt.CONSOLE, Http: HTTP) => {

  let log = (c, fmt) => Printf.ksprintf (C.log(c), fmt);

  let get_content = (c, path) =>
    log(c, "Replying: %s", path) >|= () =>
    "Hello from the httpaf unikernel"

  let dispatcher = (c, reqd) => {
    let req = Reqd.request(reqd);
    let target = req.target;
    Lwt.catch
      (() =>
         get_content (c, target) >|= (body) => {
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
module Make = (C : Mirage_types_lwt.CONSOLE, Clock : Mirage_types_lwt.PCLOCK, Http: HTTP) => {

  module D  = Dispatch (C, Http);

  let log = (c, fmt) => Printf.ksprintf (C.log(c), fmt);

  let start = (c, _clock, http) =>
    log (c, "started unikernel listen on port 8001") >>= () =>
    http ((`TCP (8001)), (D.serve (c, D.dispatcher)));
  };
