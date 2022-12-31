import { init_rpc, benchmark_server } from 'rpc/rpc';

let content = document.getElementById("content");
let test1 = document.getElementById("test1");
let test2 = document.getElementById("test2");

let total = 0;

init_rpc().then(async () => {
    {
        let t0 = performance.now()
        for (let i = 0; i < 2000; ++i) {
            let data = await benchmark_server.rpc();
            total += data.f6[50].f2;
        }
        let t1 = performance.now();
        test1.innerHTML = "Call to RPC took " + (t1 - t0) + " milliseconds.";
    }
    {
        let t0 = performance.now()
        for (let i = 0; i < 2000; ++i) {
            let str = await benchmark_server.json();
            let data = await JSON.parse(str);
            total += data.f6[50].f2;
        }
        let t1 = performance.now();
        test2.innerHTML = "Call to JSON took " + (t1 - t0) + " milliseconds.";
    }
});