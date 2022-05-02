import App from './gui/App.svelte';
import { rpc_init } from './callback'

let app: App;

rpc_init().then(() => {
	app = new App({
		target: document.body,
		props: {}
	});
}).catch((e)=>console.log(e));

export default app;