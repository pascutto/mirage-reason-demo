open Lwt.Infix;
open Httpaf;

module HTTP = Mirage_http_cohttp;

module Dispatch = (C: Mirage_types_lwt.CONSOLE, FS : Mirage_types_lwt.KV_RO, CON: Conduit_mirage.S, Http: Httpaf_mirage.Server_intf) => {

  let log = (c, fmt) => Printf.ksprintf (C.log(c), fmt);

  module Ws = Websocket.Make(HTTP)(CON);
  module Server = HTTP.Server(CON);

  let rec manager = (client) => {
    Ws.Connection.recv (client) >>= (frame) => {
      let frame = Ws.Frame.create(~content="ok.",());
      Ws.Connection.send (client, frame) >>= () => manager(client)
    }
  }

  let ws_server = (client) => {
    manager(client);
  };

  let upgrade_to_websockets = (reqd) => {
    Ws.Connection.upgrade_connection(reqd, ws_server)
  };

  let string_to_body = (str) => {
    let (stream, push) = Lwt_stream.create();
    let cs = Cstruct.of_string (str);
    push(Some ((cs,0,Cstruct.len (cs))));
    push(None);
    (() => {Lwt_stream.get (stream)})
  };

  let header_from_string = (body) =>
    HTTP.HTTP.Headers.of_list ([/*("Content-Length", body
    |> String.length
    |> string_of_int)*/])

  let get_content = (fs, c, path) =>
    FS.get(fs, Mirage_kv.Key.v (path)) >>= (res) => switch res {
      | Error (_) =>
        let body = "Not found";
        Lwt.return (
        `Response ( HTTP.Response.v (
          ~body = string_to_body (body),
          header_from_string (body),
          404
        ))
      )
      | Ok (body) => Lwt.return (
        `Response ( HTTP.Response.v (
          ~body = string_to_body (body),
          header_from_string (body),
          HTTP.s200_ok
        ))
      )
    }

  let rec handler = (fs,c,reqd) => {
    let req = Server.get_request (reqd);
    let path = Uri.path (HTTP.Request.uri (req));
    log (c, "Request: %s", path) >>= () => {
    let response = switch path {
      | "/hello" => get_content(fs, c, "/hello.html")
      | "/goodbye" => get_content(fs, c, "/goodbye.html")
      | "/" => get_content(fs, c, "/index.html")
      | "/ws" => Lwt.return (upgrade_to_websockets(req))
      | _   => get_content(fs, c, path)
    };
    response}
  };
/*
  let dispatcher = (fs, c, reqd) => {
    let req = Reqd.request(reqd);
    let target = req.target;
    Lwt.catch
      (() =>
         handler (fs, c, target) >|= (body) => {
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
  };*/

  let error_handler = () => Lwt.return_unit;

};

/* Server boilerplate */
module Make = (C : Mirage_types_lwt.CONSOLE, Clock : Mirage_types_lwt.PCLOCK, FS : Mirage_types_lwt.KV_RO, CON: Conduit_mirage.S, Http: Httpaf_mirage.Server_intf) => {

  module D  = Dispatch (C, FS, CON, Http);

  let log = (c, fmt) => Printf.ksprintf (C.log(c), fmt);

  module Server = HTTP.Server(CON);

  let start = (c, _clock, fs, conduit, _http) =>
    log (c, "started unikernel listen on port 8001") >>= () =>
    Server.connect(conduit) >>= (http) =>
    Server.create_connection_handler(D.handler(fs,c), D.error_handler) >>= (connection_handler) =>
    Server.listen(http, `TCP (8001), connection_handler);

  };
