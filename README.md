# mirage-reason-demo

To build the whole stuff:
## Front-end
* `npm install` once.
* `npm build` build the reason frontend.
* `npm run bundle` bundle the site in one js file.
## Back-end + server
* `mirage config -t unix` configure for unix target
* `mirage build` build the unikernel
* `./httpaf_unikernel` (actually it's cohttp for now..)


## Disclaimer
You need a lot of pins to make this work.
* Dunified Mirage: `opam repo add mirage-dev https://github.com/mirage/mirage-dev/archive/dune.zip`
* Httpaf for Mirage: `opam pin https://github.com/anmonteiro/httpaf.git#mirage`
* mirage-websocket: `opam pin https://github.com/TheLortex/mirage-websocket.git`
* mirage-http: `opam pin https://github.com/mirage/mirage-http.git`