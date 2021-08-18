var app = (function () {
    'use strict';

    function noop() { }
    function assign(tar, src) {
        // @ts-ignore
        for (const k in src)
            tar[k] = src[k];
        return tar;
    }
    function add_location(element, file, line, column, char) {
        element.__svelte_meta = {
            loc: { file, line, column, char }
        };
    }
    function run(fn) {
        return fn();
    }
    function blank_object() {
        return Object.create(null);
    }
    function run_all(fns) {
        fns.forEach(run);
    }
    function is_function(thing) {
        return typeof thing === 'function';
    }
    function safe_not_equal(a, b) {
        return a != a ? b == b : a !== b || ((a && typeof a === 'object') || typeof a === 'function');
    }
    function is_empty(obj) {
        return Object.keys(obj).length === 0;
    }
    function validate_store(store, name) {
        if (store != null && typeof store.subscribe !== 'function') {
            throw new Error(`'${name}' is not a store with a 'subscribe' method`);
        }
    }
    function subscribe(store, ...callbacks) {
        if (store == null) {
            return noop;
        }
        const unsub = store.subscribe(...callbacks);
        return unsub.unsubscribe ? () => unsub.unsubscribe() : unsub;
    }
    function create_slot(definition, ctx, $$scope, fn) {
        if (definition) {
            const slot_ctx = get_slot_context(definition, ctx, $$scope, fn);
            return definition[0](slot_ctx);
        }
    }
    function get_slot_context(definition, ctx, $$scope, fn) {
        return definition[1] && fn
            ? assign($$scope.ctx.slice(), definition[1](fn(ctx)))
            : $$scope.ctx;
    }
    function get_slot_changes(definition, $$scope, dirty, fn) {
        if (definition[2] && fn) {
            const lets = definition[2](fn(dirty));
            if ($$scope.dirty === undefined) {
                return lets;
            }
            if (typeof lets === 'object') {
                const merged = [];
                const len = Math.max($$scope.dirty.length, lets.length);
                for (let i = 0; i < len; i += 1) {
                    merged[i] = $$scope.dirty[i] | lets[i];
                }
                return merged;
            }
            return $$scope.dirty | lets;
        }
        return $$scope.dirty;
    }
    function update_slot_base(slot, slot_definition, ctx, $$scope, slot_changes, get_slot_context_fn) {
        if (slot_changes) {
            const slot_context = get_slot_context(slot_definition, ctx, $$scope, get_slot_context_fn);
            slot.p(slot_context, slot_changes);
        }
    }
    function get_all_dirty_from_scope($$scope) {
        if ($$scope.ctx.length > 32) {
            const dirty = [];
            const length = $$scope.ctx.length / 32;
            for (let i = 0; i < length; i++) {
                dirty[i] = -1;
            }
            return dirty;
        }
        return -1;
    }
    function append(target, node) {
        target.appendChild(node);
    }
    function insert(target, node, anchor) {
        target.insertBefore(node, anchor || null);
    }
    function detach(node) {
        node.parentNode.removeChild(node);
    }
    function destroy_each(iterations, detaching) {
        for (let i = 0; i < iterations.length; i += 1) {
            if (iterations[i])
                iterations[i].d(detaching);
        }
    }
    function element(name) {
        return document.createElement(name);
    }
    function svg_element(name) {
        return document.createElementNS('http://www.w3.org/2000/svg', name);
    }
    function text(data) {
        return document.createTextNode(data);
    }
    function space() {
        return text(' ');
    }
    function listen(node, event, handler, options) {
        node.addEventListener(event, handler, options);
        return () => node.removeEventListener(event, handler, options);
    }
    function attr(node, attribute, value) {
        if (value == null)
            node.removeAttribute(attribute);
        else if (node.getAttribute(attribute) !== value)
            node.setAttribute(attribute, value);
    }
    function children(element) {
        return Array.from(element.childNodes);
    }
    function custom_event(type, detail, bubbles = false) {
        const e = document.createEvent('CustomEvent');
        e.initCustomEvent(type, bubbles, false, detail);
        return e;
    }

    let current_component;
    function set_current_component(component) {
        current_component = component;
    }

    const dirty_components = [];
    const binding_callbacks = [];
    const render_callbacks = [];
    const flush_callbacks = [];
    const resolved_promise = Promise.resolve();
    let update_scheduled = false;
    function schedule_update() {
        if (!update_scheduled) {
            update_scheduled = true;
            resolved_promise.then(flush);
        }
    }
    function add_render_callback(fn) {
        render_callbacks.push(fn);
    }
    let flushing = false;
    const seen_callbacks = new Set();
    function flush() {
        if (flushing)
            return;
        flushing = true;
        do {
            // first, call beforeUpdate functions
            // and update components
            for (let i = 0; i < dirty_components.length; i += 1) {
                const component = dirty_components[i];
                set_current_component(component);
                update(component.$$);
            }
            set_current_component(null);
            dirty_components.length = 0;
            while (binding_callbacks.length)
                binding_callbacks.pop()();
            // then, once components are updated, call
            // afterUpdate functions. This may cause
            // subsequent updates...
            for (let i = 0; i < render_callbacks.length; i += 1) {
                const callback = render_callbacks[i];
                if (!seen_callbacks.has(callback)) {
                    // ...so guard against infinite loops
                    seen_callbacks.add(callback);
                    callback();
                }
            }
            render_callbacks.length = 0;
        } while (dirty_components.length);
        while (flush_callbacks.length) {
            flush_callbacks.pop()();
        }
        update_scheduled = false;
        flushing = false;
        seen_callbacks.clear();
    }
    function update($$) {
        if ($$.fragment !== null) {
            $$.update();
            run_all($$.before_update);
            const dirty = $$.dirty;
            $$.dirty = [-1];
            $$.fragment && $$.fragment.p($$.ctx, dirty);
            $$.after_update.forEach(add_render_callback);
        }
    }
    const outroing = new Set();
    let outros;
    function group_outros() {
        outros = {
            r: 0,
            c: [],
            p: outros // parent group
        };
    }
    function check_outros() {
        if (!outros.r) {
            run_all(outros.c);
        }
        outros = outros.p;
    }
    function transition_in(block, local) {
        if (block && block.i) {
            outroing.delete(block);
            block.i(local);
        }
    }
    function transition_out(block, local, detach, callback) {
        if (block && block.o) {
            if (outroing.has(block))
                return;
            outroing.add(block);
            outros.c.push(() => {
                outroing.delete(block);
                if (callback) {
                    if (detach)
                        block.d(1);
                    callback();
                }
            });
            block.o(local);
        }
    }
    function create_component(block) {
        block && block.c();
    }
    function mount_component(component, target, anchor, customElement) {
        const { fragment, on_mount, on_destroy, after_update } = component.$$;
        fragment && fragment.m(target, anchor);
        if (!customElement) {
            // onMount happens before the initial afterUpdate
            add_render_callback(() => {
                const new_on_destroy = on_mount.map(run).filter(is_function);
                if (on_destroy) {
                    on_destroy.push(...new_on_destroy);
                }
                else {
                    // Edge case - component was destroyed immediately,
                    // most likely as a result of a binding initialising
                    run_all(new_on_destroy);
                }
                component.$$.on_mount = [];
            });
        }
        after_update.forEach(add_render_callback);
    }
    function destroy_component(component, detaching) {
        const $$ = component.$$;
        if ($$.fragment !== null) {
            run_all($$.on_destroy);
            $$.fragment && $$.fragment.d(detaching);
            // TODO null out other refs, including component.$$ (but need to
            // preserve final state?)
            $$.on_destroy = $$.fragment = null;
            $$.ctx = [];
        }
    }
    function make_dirty(component, i) {
        if (component.$$.dirty[0] === -1) {
            dirty_components.push(component);
            schedule_update();
            component.$$.dirty.fill(0);
        }
        component.$$.dirty[(i / 31) | 0] |= (1 << (i % 31));
    }
    function init(component, options, instance, create_fragment, not_equal, props, append_styles, dirty = [-1]) {
        const parent_component = current_component;
        set_current_component(component);
        const $$ = component.$$ = {
            fragment: null,
            ctx: null,
            // state
            props,
            update: noop,
            not_equal,
            bound: blank_object(),
            // lifecycle
            on_mount: [],
            on_destroy: [],
            on_disconnect: [],
            before_update: [],
            after_update: [],
            context: new Map(parent_component ? parent_component.$$.context : options.context || []),
            // everything else
            callbacks: blank_object(),
            dirty,
            skip_bound: false,
            root: options.target || parent_component.$$.root
        };
        append_styles && append_styles($$.root);
        let ready = false;
        $$.ctx = instance
            ? instance(component, options.props || {}, (i, ret, ...rest) => {
                const value = rest.length ? rest[0] : ret;
                if ($$.ctx && not_equal($$.ctx[i], $$.ctx[i] = value)) {
                    if (!$$.skip_bound && $$.bound[i])
                        $$.bound[i](value);
                    if (ready)
                        make_dirty(component, i);
                }
                return ret;
            })
            : [];
        $$.update();
        ready = true;
        run_all($$.before_update);
        // `false` as a special case of no DOM component
        $$.fragment = create_fragment ? create_fragment($$.ctx) : false;
        if (options.target) {
            if (options.hydrate) {
                const nodes = children(options.target);
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                $$.fragment && $$.fragment.l(nodes);
                nodes.forEach(detach);
            }
            else {
                // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
                $$.fragment && $$.fragment.c();
            }
            if (options.intro)
                transition_in(component.$$.fragment);
            mount_component(component, options.target, options.anchor, options.customElement);
            flush();
        }
        set_current_component(parent_component);
    }
    /**
     * Base class for Svelte components. Used when dev=false.
     */
    class SvelteComponent {
        $destroy() {
            destroy_component(this, 1);
            this.$destroy = noop;
        }
        $on(type, callback) {
            const callbacks = (this.$$.callbacks[type] || (this.$$.callbacks[type] = []));
            callbacks.push(callback);
            return () => {
                const index = callbacks.indexOf(callback);
                if (index !== -1)
                    callbacks.splice(index, 1);
            };
        }
        $set($$props) {
            if (this.$$set && !is_empty($$props)) {
                this.$$.skip_bound = true;
                this.$$set($$props);
                this.$$.skip_bound = false;
            }
        }
    }

    function dispatch_dev(type, detail) {
        document.dispatchEvent(custom_event(type, Object.assign({ version: '3.42.1' }, detail), true));
    }
    function append_dev(target, node) {
        dispatch_dev('SvelteDOMInsert', { target, node });
        append(target, node);
    }
    function insert_dev(target, node, anchor) {
        dispatch_dev('SvelteDOMInsert', { target, node, anchor });
        insert(target, node, anchor);
    }
    function detach_dev(node) {
        dispatch_dev('SvelteDOMRemove', { node });
        detach(node);
    }
    function listen_dev(node, event, handler, options, has_prevent_default, has_stop_propagation) {
        const modifiers = options === true ? ['capture'] : options ? Array.from(Object.keys(options)) : [];
        if (has_prevent_default)
            modifiers.push('preventDefault');
        if (has_stop_propagation)
            modifiers.push('stopPropagation');
        dispatch_dev('SvelteDOMAddEventListener', { node, event, handler, modifiers });
        const dispose = listen(node, event, handler, options);
        return () => {
            dispatch_dev('SvelteDOMRemoveEventListener', { node, event, handler, modifiers });
            dispose();
        };
    }
    function attr_dev(node, attribute, value) {
        attr(node, attribute, value);
        if (value == null)
            dispatch_dev('SvelteDOMRemoveAttribute', { node, attribute });
        else
            dispatch_dev('SvelteDOMSetAttribute', { node, attribute, value });
    }
    function prop_dev(node, property, value) {
        node[property] = value;
        dispatch_dev('SvelteDOMSetProperty', { node, property, value });
    }
    function set_data_dev(text, data) {
        data = '' + data;
        if (text.wholeText === data)
            return;
        dispatch_dev('SvelteDOMSetData', { node: text, data });
        text.data = data;
    }
    function validate_each_argument(arg) {
        if (typeof arg !== 'string' && !(arg && typeof arg === 'object' && 'length' in arg)) {
            let msg = '{#each} only iterates over array-like objects.';
            if (typeof Symbol === 'function' && arg && Symbol.iterator in arg) {
                msg += ' You can use a spread to convert this iterable into an array.';
            }
            throw new Error(msg);
        }
    }
    function validate_slots(name, slot, keys) {
        for (const slot_key of Object.keys(slot)) {
            if (!~keys.indexOf(slot_key)) {
                console.warn(`<${name}> received an unexpected slot "${slot_key}".`);
            }
        }
    }
    /**
     * Base class for Svelte components with some minor dev-enhancements. Used when dev=true.
     */
    class SvelteComponentDev extends SvelteComponent {
        constructor(options) {
            if (!options || (!options.target && !options.$$inline)) {
                throw new Error("'target' is a required option");
            }
            super();
        }
        $destroy() {
            super.$destroy();
            this.$destroy = () => {
                console.warn('Component was already destroyed'); // eslint-disable-line no-console
            };
        }
        $capture_state() { }
        $inject_state() { }
    }

    /* src/Parameter.svelte generated by Svelte v3.42.1 */

    const file$3 = "src/Parameter.svelte";

    function create_fragment$3(ctx) {
    	let li;
    	let div0;
    	let t0;
    	let t1;
    	let div1;
    	let t2;
    	let t3;
    	let div2;
    	let t4;

    	const block = {
    		c: function create() {
    			li = element("li");
    			div0 = element("div");
    			t0 = text(/*name*/ ctx[0]);
    			t1 = space();
    			div1 = element("div");
    			t2 = text(/*desc*/ ctx[1]);
    			t3 = space();
    			div2 = element("div");
    			t4 = text(/*$value*/ ctx[3]);
    			attr_dev(div0, "class", "name svelte-1bf6r2k");
    			add_location(div0, file$3, 7, 1, 89);
    			attr_dev(div1, "class", "desc svelte-1bf6r2k");
    			add_location(div1, file$3, 8, 1, 121);
    			attr_dev(div2, "class", "value svelte-1bf6r2k");
    			add_location(div2, file$3, 9, 1, 153);
    			attr_dev(li, "class", "svelte-1bf6r2k");
    			add_location(li, file$3, 6, 0, 83);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, li, anchor);
    			append_dev(li, div0);
    			append_dev(div0, t0);
    			append_dev(li, t1);
    			append_dev(li, div1);
    			append_dev(div1, t2);
    			append_dev(li, t3);
    			append_dev(li, div2);
    			append_dev(div2, t4);
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*name*/ 1) set_data_dev(t0, /*name*/ ctx[0]);
    			if (dirty & /*desc*/ 2) set_data_dev(t2, /*desc*/ ctx[1]);
    			if (dirty & /*$value*/ 8) set_data_dev(t4, /*$value*/ ctx[3]);
    		},
    		i: noop,
    		o: noop,
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(li);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$3.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$3($$self, $$props, $$invalidate) {
    	let $value,
    		$$unsubscribe_value = noop,
    		$$subscribe_value = () => ($$unsubscribe_value(), $$unsubscribe_value = subscribe(value, $$value => $$invalidate(3, $value = $$value)), value);

    	$$self.$$.on_destroy.push(() => $$unsubscribe_value());
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Parameter', slots, []);
    	
    	let { name } = $$props;
    	let { desc } = $$props;
    	let { value } = $$props;
    	validate_store(value, 'value');
    	$$subscribe_value();
    	const writable_props = ['name', 'desc', 'value'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Parameter> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('name' in $$props) $$invalidate(0, name = $$props.name);
    		if ('desc' in $$props) $$invalidate(1, desc = $$props.desc);
    		if ('value' in $$props) $$subscribe_value($$invalidate(2, value = $$props.value));
    	};

    	$$self.$capture_state = () => ({ name, desc, value, $value });

    	$$self.$inject_state = $$props => {
    		if ('name' in $$props) $$invalidate(0, name = $$props.name);
    		if ('desc' in $$props) $$invalidate(1, desc = $$props.desc);
    		if ('value' in $$props) $$subscribe_value($$invalidate(2, value = $$props.value));
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [name, desc, value, $value];
    }

    class Parameter extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$3, create_fragment$3, safe_not_equal, { name: 0, desc: 1, value: 2 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Parameter",
    			options,
    			id: create_fragment$3.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*name*/ ctx[0] === undefined && !('name' in props)) {
    			console.warn("<Parameter> was created without expected prop 'name'");
    		}

    		if (/*desc*/ ctx[1] === undefined && !('desc' in props)) {
    			console.warn("<Parameter> was created without expected prop 'desc'");
    		}

    		if (/*value*/ ctx[2] === undefined && !('value' in props)) {
    			console.warn("<Parameter> was created without expected prop 'value'");
    		}
    	}

    	get name() {
    		throw new Error("<Parameter>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set name(value) {
    		throw new Error("<Parameter>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get desc() {
    		throw new Error("<Parameter>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set desc(value) {
    		throw new Error("<Parameter>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get value() {
    		throw new Error("<Parameter>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set value(value) {
    		throw new Error("<Parameter>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src/Category.svelte generated by Svelte v3.42.1 */
    const file$2 = "src/Category.svelte";

    function get_each_context$1(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[1] = list[i];
    	return child_ctx;
    }

    // (7:1) {#each cat.items as param}
    function create_each_block$1(ctx) {
    	let parameter;
    	let current;

    	parameter = new Parameter({
    			props: {
    				name: /*param*/ ctx[1].name,
    				desc: /*param*/ ctx[1].description,
    				value: /*param*/ ctx[1].ov.value
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(parameter.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(parameter, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const parameter_changes = {};
    			if (dirty & /*cat*/ 1) parameter_changes.name = /*param*/ ctx[1].name;
    			if (dirty & /*cat*/ 1) parameter_changes.desc = /*param*/ ctx[1].description;
    			if (dirty & /*cat*/ 1) parameter_changes.value = /*param*/ ctx[1].ov.value;
    			parameter.$set(parameter_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(parameter.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(parameter.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(parameter, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block$1.name,
    		type: "each",
    		source: "(7:1) {#each cat.items as param}",
    		ctx
    	});

    	return block;
    }

    function create_fragment$2(ctx) {
    	let ul;
    	let current;
    	let each_value = /*cat*/ ctx[0].items;
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block$1(get_each_context$1(ctx, each_value, i));
    	}

    	const out = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	const block = {
    		c: function create() {
    			ul = element("ul");

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			add_location(ul, file$2, 5, 0, 91);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, ul, anchor);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(ul, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*cat*/ 1) {
    				each_value = /*cat*/ ctx[0].items;
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context$1(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block$1(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(ul, null);
    					}
    				}

    				group_outros();

    				for (i = each_value.length; i < each_blocks.length; i += 1) {
    					out(i);
    				}

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;

    			for (let i = 0; i < each_value.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(ul);
    			destroy_each(each_blocks, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$2.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$2($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('Category', slots, []);
    	
    	let { cat } = $$props;
    	const writable_props = ['cat'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<Category> was created with unknown prop '${key}'`);
    	});

    	$$self.$$set = $$props => {
    		if ('cat' in $$props) $$invalidate(0, cat = $$props.cat);
    	};

    	$$self.$capture_state = () => ({ Parameter, cat });

    	$$self.$inject_state = $$props => {
    		if ('cat' in $$props) $$invalidate(0, cat = $$props.cat);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [cat];
    }

    class Category extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$2, create_fragment$2, safe_not_equal, { cat: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "Category",
    			options,
    			id: create_fragment$2.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*cat*/ ctx[0] === undefined && !('cat' in props)) {
    			console.warn("<Category> was created without expected prop 'cat'");
    		}
    	}

    	get cat() {
    		throw new Error("<Category>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set cat(value) {
    		throw new Error("<Category>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    /* src/CollapsibleSection.svelte generated by Svelte v3.42.1 */

    const file$1 = "src/CollapsibleSection.svelte";

    function create_fragment$1(ctx) {
    	let div1;
    	let h3;
    	let button;
    	let t0;
    	let t1;
    	let svg;
    	let path0;
    	let path1;
    	let t2;
    	let div0;
    	let div0_hidden_value;
    	let current;
    	let mounted;
    	let dispose;
    	const default_slot_template = /*#slots*/ ctx[3].default;
    	const default_slot = create_slot(default_slot_template, ctx, /*$$scope*/ ctx[2], null);

    	const block = {
    		c: function create() {
    			div1 = element("div");
    			h3 = element("h3");
    			button = element("button");
    			t0 = text(/*headerText*/ ctx[1]);
    			t1 = space();
    			svg = svg_element("svg");
    			path0 = svg_element("path");
    			path1 = svg_element("path");
    			t2 = space();
    			div0 = element("div");
    			if (default_slot) default_slot.c();
    			attr_dev(path0, "class", "vert svelte-1v1ukx0");
    			attr_dev(path0, "d", "M10 1V19");
    			attr_dev(path0, "stroke", "black");
    			attr_dev(path0, "stroke-width", "2");
    			add_location(path0, file$1, 11, 4, 371);
    			attr_dev(path1, "d", "M1 10L19 10");
    			attr_dev(path1, "stroke", "black");
    			attr_dev(path1, "stroke-width", "2");
    			add_location(path1, file$1, 12, 4, 441);
    			attr_dev(svg, "viewBox", "0 0 20 20");
    			attr_dev(svg, "fill", "none");
    			attr_dev(svg, "class", "svelte-1v1ukx0");
    			add_location(svg, file$1, 10, 3, 328);
    			attr_dev(button, "aria-expanded", /*expanded*/ ctx[0]);
    			attr_dev(button, "class", "svelte-1v1ukx0");
    			add_location(button, file$1, 9, 2, 241);
    			attr_dev(h3, "class", "svelte-1v1ukx0");
    			add_location(h3, file$1, 8, 1, 234);
    			attr_dev(div0, "class", "contents");
    			div0.hidden = div0_hidden_value = !/*expanded*/ ctx[0];
    			add_location(div0, file$1, 16, 1, 527);
    			attr_dev(div1, "class", "collapsible svelte-1v1ukx0");
    			add_location(div1, file$1, 7, 0, 207);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, div1, anchor);
    			append_dev(div1, h3);
    			append_dev(h3, button);
    			append_dev(button, t0);
    			append_dev(button, t1);
    			append_dev(button, svg);
    			append_dev(svg, path0);
    			append_dev(svg, path1);
    			append_dev(div1, t2);
    			append_dev(div1, div0);

    			if (default_slot) {
    				default_slot.m(div0, null);
    			}

    			current = true;

    			if (!mounted) {
    				dispose = listen_dev(button, "click", /*click_handler*/ ctx[4], false, false, false);
    				mounted = true;
    			}
    		},
    		p: function update(ctx, [dirty]) {
    			if (!current || dirty & /*headerText*/ 2) set_data_dev(t0, /*headerText*/ ctx[1]);

    			if (!current || dirty & /*expanded*/ 1) {
    				attr_dev(button, "aria-expanded", /*expanded*/ ctx[0]);
    			}

    			if (default_slot) {
    				if (default_slot.p && (!current || dirty & /*$$scope*/ 4)) {
    					update_slot_base(
    						default_slot,
    						default_slot_template,
    						ctx,
    						/*$$scope*/ ctx[2],
    						!current
    						? get_all_dirty_from_scope(/*$$scope*/ ctx[2])
    						: get_slot_changes(default_slot_template, /*$$scope*/ ctx[2], dirty, null),
    						null
    					);
    				}
    			}

    			if (!current || dirty & /*expanded*/ 1 && div0_hidden_value !== (div0_hidden_value = !/*expanded*/ ctx[0])) {
    				prop_dev(div0, "hidden", div0_hidden_value);
    			}
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(default_slot, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(default_slot, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(div1);
    			if (default_slot) default_slot.d(detaching);
    			mounted = false;
    			dispose();
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment$1.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance$1($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('CollapsibleSection', slots, ['default']);
    	let { headerText } = $$props;
    	let { expanded = false } = $$props;
    	const writable_props = ['headerText', 'expanded'];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<CollapsibleSection> was created with unknown prop '${key}'`);
    	});

    	const click_handler = () => $$invalidate(0, expanded = !expanded);

    	$$self.$$set = $$props => {
    		if ('headerText' in $$props) $$invalidate(1, headerText = $$props.headerText);
    		if ('expanded' in $$props) $$invalidate(0, expanded = $$props.expanded);
    		if ('$$scope' in $$props) $$invalidate(2, $$scope = $$props.$$scope);
    	};

    	$$self.$capture_state = () => ({ headerText, expanded });

    	$$self.$inject_state = $$props => {
    		if ('headerText' in $$props) $$invalidate(1, headerText = $$props.headerText);
    		if ('expanded' in $$props) $$invalidate(0, expanded = $$props.expanded);
    	};

    	if ($$props && "$$inject" in $$props) {
    		$$self.$inject_state($$props.$$inject);
    	}

    	return [expanded, headerText, $$scope, slots, click_handler];
    }

    class CollapsibleSection extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance$1, create_fragment$1, safe_not_equal, { headerText: 1, expanded: 0 });

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "CollapsibleSection",
    			options,
    			id: create_fragment$1.name
    		});

    		const { ctx } = this.$$;
    		const props = options.props || {};

    		if (/*headerText*/ ctx[1] === undefined && !('headerText' in props)) {
    			console.warn("<CollapsibleSection> was created without expected prop 'headerText'");
    		}
    	}

    	get headerText() {
    		throw new Error("<CollapsibleSection>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set headerText(value) {
    		throw new Error("<CollapsibleSection>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	get expanded() {
    		throw new Error("<CollapsibleSection>: Props cannot be read directly from the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}

    	set expanded(value) {
    		throw new Error("<CollapsibleSection>: Props cannot be set directly on the component instance unless compiling with 'accessors: true' or '<svelte:options accessors/>'");
    	}
    }

    function createCommonjsModule(fn) {
      var module = { exports: {} };
    	return fn(module, module.exports), module.exports;
    }

    /*
     * ATTENTION: The "eval" devtool has been used (maybe by default in mode: "development").
     * This devtool is neither made for production nor for readable output files.
     * It uses "eval()" calls to create a separate source file in the browser devtools.
     * If you are trying to read the output file, select a different devtool (https://webpack.js.org/configuration/devtool/)
     * or disable the default devtool with "devtool: false".
     * If you are looking for production-ready output files, see mode: "production" (https://webpack.js.org/configuration/mode/).
     */

    var nprpc = createCommonjsModule(function (module, exports) {
    (function webpackUniversalModuleDefinition(root, factory) {
    	module.exports = factory();
    })(self, function() {
    return /******/ (() => { // webpackBootstrap
    /******/ 	var __webpack_modules__ = ({

    /***/ "./src/exception.ts":
    /*!**************************!*\
      !*** ./src/exception.ts ***!
      \**************************/
    /***/ ((__unused_webpack_module, exports) => {

    eval("\n// Copyright (c) 2021 nikitapnn1@gmail.com\n// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory\nObject.defineProperty(exports, \"__esModule\", ({ value: true }));\nexports.Exception = void 0;\nclass Exception extends Error {\n    constructor(message) {\n        super(message);\n    }\n}\nexports.Exception = Exception;\n\n\n//# sourceURL=webpack://nprpc_runtime/./src/exception.ts?");

    /***/ }),

    /***/ "./src/flat.ts":
    /*!*********************!*\
      !*** ./src/flat.ts ***!
      \*********************/
    /***/ ((__unused_webpack_module, exports) => {

    eval("\n// Copyright (c) 2021 nikitapnn1@gmail.com\n// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory\nObject.defineProperty(exports, \"__esModule\", ({ value: true }));\nexports.Array_Direct1_float64 = exports.Array_Direct1_float32 = exports.Array_Direct1_i64 = exports.Array_Direct1_u64 = exports.Array_Direct1_i32 = exports.Array_Direct1_u32 = exports.Array_Direct1_i16 = exports.Array_Direct1_u16 = exports.Array_Direct1_i8 = exports.Array_Direct1_u8 = exports.Vector_Direct1_float64 = exports.Vector_Direct1_float32 = exports.Vector_Direct1_i64 = exports.Vector_Direct1_u64 = exports.Vector_Direct1_i32 = exports.Vector_Direct1_u32 = exports.Vector_Direct1_i16 = exports.Vector_Direct1_u16 = exports.Vector_Direct1_i8 = exports.Vector_Direct1_u8 = exports.String_Direct1 = exports.Array_Direct2 = exports.Vector_Direct2 = exports._alloc = exports.Flat = void 0;\nclass Flat {\n    buffer;\n    offset;\n    constructor(buffer, offset) {\n        this.buffer = buffer;\n        this.offset = offset;\n    }\n}\nexports.Flat = Flat;\nfunction _alloc(buffer, vector_offset, n, element_size, align) {\n    let rem = buffer.offset % align;\n    let offset = rem ? buffer.offset + (align - rem) : buffer.offset;\n    {\n        let added_size = n * element_size + (offset - buffer.offset);\n        buffer.prepare(added_size);\n        buffer.commit(added_size);\n    }\n    buffer.dv.setUint32(vector_offset, offset - vector_offset, true);\n    buffer.dv.setUint32(vector_offset + 4, n, true);\n    //\tconsole.log({\n    //\t\toffset: offset,\n    //\t\tvector_offset: vector_offset,\n    //\t\tvoffset: buffer.dv.getUint32(vector_offset,  true)\n    //\t});\n    return offset;\n}\nexports._alloc = _alloc;\nclass Vector extends Flat {\n    get elements_offset() { return this.offset + this.buffer.dv.getUint32(this.offset, true); }\n    get elements_size() { return this.buffer.dv.getUint32(this.offset + 4, true); }\n}\nclass FlatArray extends Flat {\n    buffer;\n    offset;\n    size;\n    get elements_offset() { return this.offset; }\n    get elements_size() { return this.size; }\n    constructor(buffer, offset, size) {\n        super(buffer, offset);\n        this.buffer = buffer;\n        this.offset = offset;\n        this.size = size;\n    }\n}\nfunction Iterable(Base) {\n    return class _Iterable extends Base {\n        // *[Symbol.iterator]() {\n        // \tlet size: number = (this as any).elements_size;\n        // \tfor (let ix = 0; ix < size; ++ix) {\n        // \t\tyield (this as any).get_val(ix);\n        // \t}\n        // }\n        [Symbol.iterator]() {\n            let this_ = this;\n            const size = this_.elements_size;\n            let index = 0;\n            return {\n                next: () => {\n                    if (index < size)\n                        return { value: this_.get_val(index++), done: false };\n                    return { value: size, done: true };\n                }\n            };\n        }\n    };\n}\nfunction Accessor_U8(Base) {\n    return class _Accessor_U8 extends Base {\n        get_val(ix) { return this.buffer.dv.getUint8(this.elements_offset + ix); }\n        set_val(ix, value) { this.buffer.dv.setUint8(this.elements_offset + ix, value); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setUint8(offset, n);\n                offset += 1;\n            }\n        }\n        get array_buffer() {\n            let offset = this.elements_offset;\n            return this.buffer.array_buffer.slice(offset, offset + this.elements_size);\n        }\n        get data_view() {\n            return new DataView(this.buffer.array_buffer, this.elements_offset);\n        }\n    };\n}\nfunction Accessor_I8(Base) {\n    return class _Accessor_I8 extends Base {\n        get_val(ix) { return this.buffer.dv.getInt8(this.elements_offset + ix); }\n        set_val(ix, value) { this.buffer.dv.setInt8(this.elements_offset + ix, value); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setInt8(offset, n);\n                offset += 1;\n            }\n        }\n    };\n}\nfunction Accessor_U16(Base) {\n    return class _Accessor_U16 extends Base {\n        get_val(ix) { return this.buffer.dv.getUint16(this.elements_offset + 2 * ix, true); }\n        set_val(ix, value) { this.buffer.dv.setUint16(this.elements_offset + 2 * ix, value, true); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setUint16(offset, n, true);\n                offset += 2;\n            }\n        }\n    };\n}\nfunction Accessor_I16(Base) {\n    return class _Accessor_I16 extends Base {\n        get_val(ix) { return this.buffer.dv.getInt16(this.elements_offset + 2 * ix, true); }\n        set_val(ix, value) { this.buffer.dv.setInt16(this.elements_offset + 2 * ix, value, true); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setInt16(offset, n, true);\n                offset += 2;\n            }\n        }\n    };\n}\nfunction Accessor_U32(Base) {\n    return class _Accessor_U32 extends Base {\n        get_val(ix) { return this.buffer.dv.getUint32(this.elements_offset + 4 * ix, true); }\n        set_val(ix, value) { this.buffer.dv.setUint32(this.elements_offset + 4 * ix, value, true); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setUint32(offset, n, true);\n                offset += 4;\n            }\n        }\n    };\n}\nfunction Accessor_I32(Base) {\n    return class _Accessor_I32 extends Base {\n        get_val(ix) { return this.buffer.dv.getInt32(this.elements_offset + 4 * ix, true); }\n        set_val(ix, value) { this.buffer.dv.setInt32(this.elements_offset + 4 * ix, value, true); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setInt32(offset, n, true);\n                offset += 4;\n            }\n        }\n    };\n}\nfunction Accessor_U64(Base) {\n    return class _Accessor_U64 extends Base {\n        get_val(ix) { return this.buffer.dv.getBigUint64(this.elements_offset + 8 * ix, true); }\n        set_val(ix, value) { this.buffer.dv.setBigUint64(this.elements_offset + 8 * ix, value, true); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setBigUint64(offset, n, true);\n                offset += 8;\n            }\n        }\n    };\n}\nfunction Accessor_I64(Base) {\n    return class _Accessor_I64 extends Base {\n        get_val(ix) { return this.buffer.dv.getBigInt64(this.elements_offset + 8 * ix, true); }\n        set_val(ix, value) { this.buffer.dv.setBigInt64(this.elements_offset + 8 * ix, value, true); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setBigInt64(offset, n, true);\n                offset += 8;\n            }\n        }\n    };\n}\nfunction Accessor_Float32(Base) {\n    return class _Accessor_Float32 extends Base {\n        get_val(ix) { return this.buffer.dv.getFloat32(this.elements_offset + 4 * ix, true); }\n        set_val(ix, value) { this.buffer.dv.setFloat32(this.elements_offset + 4 * ix, value, true); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setFloat32(offset, n, true);\n                offset += 4;\n            }\n        }\n    };\n}\nfunction Accessor_Float64(Base) {\n    return class _Accessor_Float64 extends Base {\n        get_val(ix) { return this.buffer.dv.getFloat64(this.elements_offset + 8 * ix, true); }\n        set_val(ix, value) { this.buffer.dv.setFloat64(this.elements_offset + 8 * ix, value, true); }\n        copy_from_ts_array(arr) {\n            let offset = this.elements_offset;\n            for (let n of arr) {\n                this.buffer.dv.setFloat64(offset, n, true);\n                offset += 8;\n            }\n        }\n    };\n}\nclass Vector_Direct2 extends Vector {\n    element_size;\n    flat_struct_constructor;\n    *[Symbol.iterator]() {\n        let size = this.elements_size;\n        for (let ix = 0; ix < size; ++ix) {\n            yield new this.flat_struct_constructor(this.buffer, this.elements_offset + this.element_size * ix);\n        }\n    }\n    constructor(buffer, offset, element_size, flat_struct_constructor) {\n        super(buffer, offset);\n        this.element_size = element_size;\n        this.flat_struct_constructor = flat_struct_constructor;\n    }\n}\nexports.Vector_Direct2 = Vector_Direct2;\nclass Array_Direct2 extends FlatArray {\n    element_size;\n    flat_struct_constructor;\n    *[Symbol.iterator]() {\n        let size = this.elements_size;\n        for (let ix = 0; ix < size; ++ix) {\n            yield new this.flat_struct_constructor(this.buffer, this.elements_offset + this.element_size * ix);\n        }\n    }\n    constructor(buffer, offset, element_size, flat_struct_constructor, size) {\n        super(buffer, offset, size);\n        this.element_size = element_size;\n        this.flat_struct_constructor = flat_struct_constructor;\n    }\n}\nexports.Array_Direct2 = Array_Direct2;\nclass String_Direct1 extends Iterable(Accessor_U8(Vector)) {\n    assign(str) {\n        let utf8_string = new TextEncoder().encode(str);\n        let offset = _alloc(this.buffer, this.offset, utf8_string.length, 1, 1);\n        new Uint8Array(this.buffer.array_buffer, offset).set(utf8_string);\n    }\n}\nexports.String_Direct1 = String_Direct1;\nclass Vector_Direct1_u8 extends Iterable(Accessor_U8(Vector)) {\n}\nexports.Vector_Direct1_u8 = Vector_Direct1_u8;\nclass Vector_Direct1_i8 extends Iterable(Accessor_I8(Vector)) {\n}\nexports.Vector_Direct1_i8 = Vector_Direct1_i8;\nclass Vector_Direct1_u16 extends Iterable(Accessor_U16(Vector)) {\n}\nexports.Vector_Direct1_u16 = Vector_Direct1_u16;\nclass Vector_Direct1_i16 extends Iterable(Accessor_I16(Vector)) {\n}\nexports.Vector_Direct1_i16 = Vector_Direct1_i16;\nclass Vector_Direct1_u32 extends Iterable(Accessor_U32(Vector)) {\n}\nexports.Vector_Direct1_u32 = Vector_Direct1_u32;\nclass Vector_Direct1_i32 extends Iterable(Accessor_I32(Vector)) {\n}\nexports.Vector_Direct1_i32 = Vector_Direct1_i32;\nclass Vector_Direct1_u64 extends Iterable(Accessor_U64(Vector)) {\n}\nexports.Vector_Direct1_u64 = Vector_Direct1_u64;\nclass Vector_Direct1_i64 extends Iterable(Accessor_I64(Vector)) {\n}\nexports.Vector_Direct1_i64 = Vector_Direct1_i64;\nclass Vector_Direct1_float32 extends Iterable(Accessor_Float32(Vector)) {\n}\nexports.Vector_Direct1_float32 = Vector_Direct1_float32;\nclass Vector_Direct1_float64 extends Iterable(Accessor_Float64(Vector)) {\n}\nexports.Vector_Direct1_float64 = Vector_Direct1_float64;\nclass Array_Direct1_u8 extends Iterable(Accessor_U8(FlatArray)) {\n}\nexports.Array_Direct1_u8 = Array_Direct1_u8;\nclass Array_Direct1_i8 extends Iterable(Accessor_I8(FlatArray)) {\n}\nexports.Array_Direct1_i8 = Array_Direct1_i8;\nclass Array_Direct1_u16 extends Iterable(Accessor_U16(FlatArray)) {\n}\nexports.Array_Direct1_u16 = Array_Direct1_u16;\nclass Array_Direct1_i16 extends Iterable(Accessor_I16(FlatArray)) {\n}\nexports.Array_Direct1_i16 = Array_Direct1_i16;\nclass Array_Direct1_u32 extends Iterable(Accessor_U32(FlatArray)) {\n}\nexports.Array_Direct1_u32 = Array_Direct1_u32;\nclass Array_Direct1_i32 extends Iterable(Accessor_I32(FlatArray)) {\n}\nexports.Array_Direct1_i32 = Array_Direct1_i32;\nclass Array_Direct1_u64 extends Iterable(Accessor_U64(FlatArray)) {\n}\nexports.Array_Direct1_u64 = Array_Direct1_u64;\nclass Array_Direct1_i64 extends Iterable(Accessor_I64(FlatArray)) {\n}\nexports.Array_Direct1_i64 = Array_Direct1_i64;\nclass Array_Direct1_float32 extends Iterable(Accessor_Float32(FlatArray)) {\n}\nexports.Array_Direct1_float32 = Array_Direct1_float32;\nclass Array_Direct1_float64 extends Iterable(Accessor_Float64(FlatArray)) {\n}\nexports.Array_Direct1_float64 = Array_Direct1_float64;\n\n\n//# sourceURL=webpack://nprpc_runtime/./src/flat.ts?");

    /***/ }),

    /***/ "./src/flat_buffer.ts":
    /*!****************************!*\
      !*** ./src/flat_buffer.ts ***!
      \****************************/
    /***/ ((__unused_webpack_module, exports) => {

    eval("\nObject.defineProperty(exports, \"__esModule\", ({ value: true }));\nexports.FlatBuffer = void 0;\nclass FlatBuffer {\n    capacity;\n    size_;\n    array_buffer;\n    dv;\n    static from_array_buffer(array_buffer) {\n        let b = new FlatBuffer();\n        b.array_buffer = array_buffer;\n        b.size_ = b.capacity = array_buffer.byteLength;\n        b.dv = new DataView(b.array_buffer);\n        return b;\n    }\n    static create(initial_capacity = 512) {\n        let b = new FlatBuffer();\n        b.capacity = initial_capacity;\n        b.size_ = 0;\n        b.array_buffer = new ArrayBuffer(initial_capacity);\n        b.dv = new DataView(b.array_buffer);\n        return b;\n    }\n    prepare(n) {\n        if (this.size_ + n > this.capacity) {\n            this.capacity = Math.max(this.capacity * 2, this.capacity + n);\n            let new_buffer = new ArrayBuffer(this.capacity);\n            new Uint8Array(new_buffer).set(new Uint8Array(this.array_buffer));\n            this.array_buffer = new_buffer;\n            this.dv = new DataView(this.array_buffer);\n        }\n    }\n    consume(n) { this.size_ -= n; }\n    commit(n) { this.size_ += n; }\n    get offset() { return this.size_; }\n    get size() { return this.size_; }\n    write_len(msg_len) {\n        this.dv.setUint32(0, msg_len, true);\n    }\n    write_msg_id(msg) {\n        this.dv.setUint32(4, msg, true);\n    }\n    read_msg_id() {\n        return this.dv.getUint32(4, true);\n    }\n    write_msg_type(msg) {\n        this.dv.setUint32(8, msg, true);\n    }\n    read_msg_type() {\n        return this.dv.getUint32(8, true);\n    }\n    read_exception_number() {\n        return this.dv.getUint32(16, true);\n    }\n    get writable_view() {\n        return new DataView(this.array_buffer, 0, this.size);\n    }\n    set_buffer(abuf) {\n        this.array_buffer = abuf;\n        this.size_ = this.capacity = abuf.byteLength;\n        this.dv = new DataView(this.array_buffer);\n    }\n    dump() {\n        let s = new String();\n        new Uint8Array(this.array_buffer, 0, this.size).forEach((x) => s += x.toString(16) + ' ');\n        console.log(s);\n    }\n}\nexports.FlatBuffer = FlatBuffer;\n\n\n//# sourceURL=webpack://nprpc_runtime/./src/flat_buffer.ts?");

    /***/ }),

    /***/ "./src/index.ts":
    /*!**********************!*\
      !*** ./src/index.ts ***!
      \**********************/
    /***/ (function(__unused_webpack_module, exports, __webpack_require__) {

    eval("\n// Copyright (c) 2021 nikitapnn1@gmail.com\n// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory\nvar __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {\n    if (k2 === undefined) k2 = k;\n    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });\n}) : (function(o, m, k, k2) {\n    if (k2 === undefined) k2 = k;\n    o[k2] = m[k];\n}));\nvar __exportStar = (this && this.__exportStar) || function(m, exports) {\n    for (var p in m) if (p !== \"default\" && !Object.prototype.hasOwnProperty.call(exports, p)) __createBinding(exports, m, p);\n};\nObject.defineProperty(exports, \"__esModule\", ({ value: true }));\nexports.get_nameserver = void 0;\n__exportStar(__webpack_require__(/*! ./nprpc */ \"./src/nprpc.ts\"), exports);\n__exportStar(__webpack_require__(/*! ./nprpc_nameserver */ \"./src/nprpc_nameserver.ts\"), exports);\nconst nprpc_nameserver_1 = __webpack_require__(/*! ./nprpc_nameserver */ \"./src/nprpc_nameserver.ts\");\nfunction sip4_to_u32(str) {\n    let rx = /(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)/ig;\n    let parts = rx.exec(str);\n    if (parts.length != 5)\n        throw \"ip address is incorrect\";\n    return Number(parts[1]) << 24 | Number(parts[2]) << 16 | Number(parts[3]) << 8 | Number(parts[4]);\n}\nfunction get_nameserver(nameserver_ip) {\n    let obj = new nprpc_nameserver_1.Nameserver();\n    obj.data.ip4 = sip4_to_u32(nameserver_ip);\n    obj.data.port = 15000;\n    obj.data.websocket_port = 15001;\n    obj.data.poa_idx = 0;\n    obj.data.object_id = 0n;\n    obj.data.flags = 0;\n    obj.data.class_id = nprpc_nameserver_1._INameserver_Servant._get_class();\n    return obj;\n}\nexports.get_nameserver = get_nameserver;\n\n\n//# sourceURL=webpack://nprpc_runtime/./src/index.ts?");

    /***/ }),

    /***/ "./src/nprpc.ts":
    /*!**********************!*\
      !*** ./src/nprpc.ts ***!
      \**********************/
    /***/ (function(__unused_webpack_module, exports, __webpack_require__) {

    eval("\nvar __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {\n    if (k2 === undefined) k2 = k;\n    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });\n}) : (function(o, m, k, k2) {\n    if (k2 === undefined) k2 = k;\n    o[k2] = m[k];\n}));\nvar __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {\n    Object.defineProperty(o, \"default\", { enumerable: true, value: v });\n}) : function(o, v) {\n    o[\"default\"] = v;\n});\nvar __importStar = (this && this.__importStar) || function (mod) {\n    if (mod && mod.__esModule) return mod;\n    var result = {};\n    if (mod != null) for (var k in mod) if (k !== \"default\" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);\n    __setModuleDefault(result, mod);\n    return result;\n};\nvar __exportStar = (this && this.__exportStar) || function(m, exports) {\n    for (var p in m) if (p !== \"default\" && !Object.prototype.hasOwnProperty.call(exports, p)) __createBinding(exports, m, p);\n};\nObject.defineProperty(exports, \"__esModule\", ({ value: true }));\nexports.set_debug_level = exports.init = exports.rpc = exports.create_object_from_flat = exports.narrow = exports.oid_create_from_flat = exports.handle_standart_reply = exports.ReferenceList = exports.make_simple_answer = exports.ObjectServant = exports.ObjectProxy = exports.Poa = exports.Rpc = exports.Connection = exports.make_ref = exports.FlatBuffer = exports.Flat = void 0;\nconst Flat = __importStar(__webpack_require__(/*! ./flat */ \"./src/flat.ts\"));\nexports.Flat = Flat;\nconst flat_buffer_1 = __webpack_require__(/*! ./flat_buffer */ \"./src/flat_buffer.ts\");\nObject.defineProperty(exports, \"FlatBuffer\", ({ enumerable: true, get: function () { return flat_buffer_1.FlatBuffer; } }));\n__exportStar(__webpack_require__(/*! ./exception */ \"./src/exception.ts\"), exports);\n__exportStar(__webpack_require__(/*! ./nprpc */ \"./src/nprpc.ts\"), exports);\n__exportStar(__webpack_require__(/*! ./nprpc_base */ \"./src/nprpc_base.ts\"), exports);\nconst exception_1 = __webpack_require__(/*! ./exception */ \"./src/exception.ts\");\n//import { FlatBuffer } from \"./flat_buffer\";\nconst nprpc_base_1 = __webpack_require__(/*! ./nprpc_base */ \"./src/nprpc_base.ts\");\nconst header_size = 16;\nlet g_debug_level = 0 /* DebugLevel_Critical */;\n;\nfunction make_ref() {\n    return { value: undefined };\n}\nexports.make_ref = make_ref;\nclass MyPromise {\n    actual_promise_;\n    resolve_;\n    reject_;\n    constructor() {\n        this.actual_promise_ = new Promise((resolve, reject) => {\n            this.resolve_ = (x) => { resolve(x); };\n            this.reject_ = (x) => { reject(x); };\n        });\n    }\n    get $() { return this.actual_promise_; }\n    set_promise(value) {\n        this.resolve_(value);\n    }\n    set_exception(ec) {\n        this.reject_(ec);\n    }\n}\nfunction get_object(buffer, poa_idx, object_id) {\n    do {\n        let poa = exports.rpc.get_poa(poa_idx);\n        if (!poa) {\n            make_simple_answer(buffer, 6 /* Error_PoaNotExist */);\n            console.log(\"Bad poa index. \" + poa_idx);\n            break;\n        }\n        let obj = poa.get_object(object_id);\n        if (!obj) {\n            make_simple_answer(buffer, 7 /* Error_ObjectNotExist */);\n            console.log(\"Object not found. \" + object_id);\n            break;\n        }\n        return obj;\n    } while (true);\n    return null;\n}\nclass Connection {\n    endpoint;\n    ws;\n    queue;\n    async perform_one() {\n        this.ws.send(this.queue[0].buffer.writable_view);\n    }\n    on_open() {\n        if (this.queue.length)\n            this.perform_one();\n    }\n    async send_receive(buffer, timeout_ms) {\n        let promise = new MyPromise();\n        this.queue.push({ buffer: buffer, promise: promise });\n        if (this.ws.readyState && this.queue.length == 1)\n            this.perform_one();\n        return promise.$;\n    }\n    on_read(ev) {\n        let buf = flat_buffer_1.FlatBuffer.from_array_buffer(ev.data);\n        if (buf.read_msg_type() == 1 /* Answer */) {\n            let task = this.queue[0];\n            task.buffer.set_buffer(ev.data);\n            this.queue.shift();\n            task.promise.set_promise();\n            if (this.queue.length)\n                this.perform_one();\n        }\n        else {\n            switch (buf.read_msg_id()) {\n                case 0 /* FunctionCall */: {\n                    let ch = new nprpc_base_1.impl.Flat_nprpc_base.CallHeader_Direct(buf, header_size);\n                    if (g_debug_level >= 2 /* DebugLevel_EveryCall */) {\n                        console.log(\"FunctionCall. interface_idx: \" + ch.interface_idx + \" , fn_idx: \" + ch.function_idx\n                            + \" , poa_idx: \" + ch.poa_idx + \" , oid: \" + ch.object_id);\n                    }\n                    let obj = get_object(buf, ch.poa_idx, ch.object_id);\n                    if (obj) {\n                        //console.log(obj);\n                        obj.dispatch(buf, this.endpoint, false);\n                    }\n                    break;\n                }\n                case 2 /* AddReference */: {\n                    let msg = new nprpc_base_1.detail.Flat_nprpc_base.ObjectIdLocal_Direct(buf, header_size);\n                    //detail::ObjectIdLocal oid{ msg.poa_idx(), msg.object_id() };\n                    if (g_debug_level >= 2 /* DebugLevel_EveryCall */) {\n                        console.log(\"AddReference. poa_idx: \" + msg.poa_idx + \" , oid: \" + msg.object_id);\n                    }\n                    let obj = get_object(buf, msg.poa_idx, msg.object_id);\n                    if (obj) {\n                        //\tif (g_cfg.debug_level >= DebugLevel_EveryCall) {\n                        //\t\tstd::cout << \"Refference added.\" << std::endl;\n                        //\t}\n                        make_simple_answer(buf, 4 /* Success */);\n                    }\n                    break;\n                }\n                case 3 /* ReleaseObject */: {\n                    let msg = new nprpc_base_1.detail.Flat_nprpc_base.ObjectIdLocal_Direct(buf, header_size);\n                    //detail::ObjectIdLocal oid{ msg.poa_idx(), msg.object_id() };\n                    //if (g_cfg.debug_level >= DebugLevel_EveryCall) {\n                    //\tstd::cout << \"ReleaseObject. \" << \"poa_idx: \" << oid.poa_idx << \", oid: \" << oid.object_id << std::endl;\n                    //}\n                    //if (ref_list_.remove_ref(msg.poa_idx(), msg.object_id())) {\n                    //\tmake_simple_answer(rx_buffer_(), nprpc::impl::MessageId::Success);\n                    //} else {\n                    //\tmake_simple_answer(rx_buffer_(), nprpc::impl::MessageId::Error_ObjectNotExist);\n                    //}\n                    break;\n                }\n                default:\n                    make_simple_answer(buf, 10 /* Error_UnknownMessageId */);\n                    break;\n            }\n            this.ws.send(buf.writable_view);\n        }\n    }\n    on_close() { }\n    on_error(ev) {\n        ///\tif (buf.read_msg_type() == impl.MessageType.Answer) {\n        //\t\tlet task = this.queue[0];\n        //\t\ttask.buffer.set_buffer(ev.data as ArrayBuffer);\n        //\t\tthis.queue.shift();\n        //\t\ttask.promise.set_promise();\n        //\t}\n    }\n    constructor(endpoint) {\n        this.endpoint = endpoint;\n        this.queue = new Array();\n        let ip4tostr = (ip4) => (ip4 >> 24 & 0xFF) + '.' + (ip4 >> 16 & 0xFF) + '.' + (ip4 >> 8 & 0xFF) + '.' + (ip4 & 0xFF);\n        this.ws = new WebSocket('ws://' + ip4tostr(this.endpoint.ip4) + ':' + this.endpoint.port.toString(10));\n        this.ws.binaryType = 'arraybuffer';\n        this.ws.onopen = this.on_open.bind(this);\n        this.ws.onclose = this.on_close.bind(this);\n        this.ws.onmessage = this.on_read.bind(this);\n        this.ws.onerror = this.on_error.bind(this);\n    }\n}\nexports.Connection = Connection;\nclass Rpc {\n    /** @internal */\n    last_poa_id_;\n    /** @internal */\n    opened_connections_;\n    /** @internal */\n    poa_list_;\n    get_connection(endpoint) {\n        let founded = this.opened_connections_.find(c => c.endpoint.ip4 === endpoint.ip4 && c.endpoint.port == endpoint.port);\n        if (founded)\n            return founded;\n        let con = new Connection(endpoint);\n        this.opened_connections_.push(con);\n        return con;\n    }\n    create_poa(poa_size) {\n        let poa = new Poa(this.last_poa_id_++, poa_size);\n        this.poa_list_.push(poa);\n        return poa;\n    }\n    get_poa(poa_idx) {\n        if (poa_idx >= this.poa_list_.length || poa_idx < 0)\n            return null;\n        return this.poa_list_[poa_idx];\n    }\n    async call(endpoint, buffer, timeout_ms = 2500) {\n        return this.get_connection(endpoint).send_receive(buffer, timeout_ms);\n    }\n    constructor() {\n        this.last_poa_id_ = 0;\n        this.opened_connections_ = new Array();\n        this.poa_list_ = new Array();\n    }\n}\nexports.Rpc = Rpc;\nconst index = (oid) => {\n    return Number(0xffffffffn & oid);\n};\nconst generation_index = (oid) => {\n    return Number(oid >> 32n);\n};\nclass Storage {\n    max_size_;\n    data_;\n    tail_idx_;\n    add(val) {\n        let old_free_idx = this.tail_idx_;\n        if (old_free_idx == this.max_size_)\n            return invalid_object_id;\n        let old_free = this.data_[this.tail_idx_];\n        this.tail_idx_ = index(old_free.oid); // next free\n        old_free.obj = val;\n        return BigInt(old_free_idx) | (BigInt(generation_index(old_free.oid)) << 32n);\n    }\n    remove(oid) {\n        let idx = index(oid);\n        this.data_[idx].oid = BigInt(this.tail_idx_) | BigInt(generation_index(oid) + 1);\n        this.data_[idx].obj = null;\n        this.tail_idx_ = idx;\n    }\n    get(oid) {\n        let idx = index(oid);\n        if (idx >= this.max_size_)\n            return null;\n        let obj = this.data_[idx];\n        if (generation_index(obj.oid) != generation_index(oid))\n            return null;\n        return obj.obj;\n    }\n    constructor(max_size) {\n        this.max_size_ = max_size;\n        this.data_ = new Array(this.max_size_);\n        for (let i = 0; i < this.max_size_; ++i) {\n            this.data_[i] = { oid: BigInt(i + 1), obj: null };\n        }\n        Object.seal(this.data_);\n        this.tail_idx_ = 0;\n    }\n}\nclass Poa {\n    /** @internal */\n    object_map_;\n    /** @internal */\n    index_;\n    get index() { return this.index_; }\n    get_object(oid) {\n        return this.object_map_.get(oid);\n    }\n    activate_object(obj) {\n        //console.log({obj: obj});\n        obj.poa_ = this;\n        obj.activation_time_ = Date.now();\n        let object_id_internal = this.object_map_.add(obj);\n        if (object_id_internal === invalid_object_id)\n            throw new exception_1.Exception(\"Poa fixed size has been exceeded\");\n        obj.object_id_ = object_id_internal;\n        obj.ref_cnt_ = 0;\n        let oid = {\n            object_id: object_id_internal,\n            ip4: localhost_ip4,\n            port: 0,\n            websocket_port: 0,\n            poa_idx: this.index,\n            flags: (1 << 1 /* WebObject */),\n            class_id: obj.get_class()\n        };\n        //if (pl_lifespan == Policy_Lifespan::Type::Transient) {\n        //\tstd::lock_guard<std::mutex> lk(g_orb->new_activated_objects_mut_);\n        //\tg_orb->new_activated_objects_.push_back(obj);\n        //}\n        return oid;\n    }\n    deactivate_object(object_id) {\n        //auto obj = object_map_.get(object_id);\n        //if (obj) {\n        //\tobj->to_delete_.store(true);\n        //\tobject_map_.remove(object_id);\n        //} else {\n        //\tstd::cerr << \"deactivate_object: object not found. id = \" << object_id << '\\n';\n        //}\n    }\n    constructor(index, max_size) {\n        this.index_ = index;\n        this.object_map_ = new Storage(max_size);\n    }\n}\nexports.Poa = Poa;\nclass ObjectProxy {\n    data;\n    /** @internal */\n    local_ref_cnt_;\n    /** @internal */\n    timeout_ms_;\n    constructor(data) {\n        if (!data)\n            this.data = new Object();\n        else\n            this.data = data;\n        this.timeout_ms_ = 1000;\n        this.local_ref_cnt_ = 0;\n    }\n    get timeout() { return this.timeout_ms_; }\n    add_ref() {\n        this.local_ref_cnt_++;\n        if (this.local_ref_cnt_ != 1)\n            return this.local_ref_cnt_;\n        const msg_size = header_size + 16;\n        let buf = flat_buffer_1.FlatBuffer.create(msg_size);\n        buf.write_len(msg_size - 4);\n        buf.write_msg_id(2 /* AddReference */);\n        buf.write_msg_type(0 /* Request */);\n        let msg = new nprpc_base_1.detail.Flat_nprpc_base.ObjectIdLocal_Direct(buf, header_size);\n        msg.poa_idx = this.data.poa_idx;\n        msg.object_id = this.data.object_id;\n        buf.commit(msg_size);\n        exports.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout).then().catch();\n        return this.local_ref_cnt_;\n    }\n    release() {\n        if (--this.local_ref_cnt_ != 0)\n            return this.local_ref_cnt_;\n        const msg_size = header_size + 16;\n        let buf = flat_buffer_1.FlatBuffer.create(msg_size);\n        buf.write_len(msg_size - 4);\n        buf.write_msg_id(3 /* ReleaseObject */);\n        buf.write_msg_type(0 /* Request */);\n        let msg = new nprpc_base_1.detail.Flat_nprpc_base.ObjectIdLocal_Direct(buf, header_size);\n        msg.poa_idx = this.data.poa_idx;\n        msg.object_id = this.data.object_id;\n        buf.commit(msg_size);\n        exports.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout).then().catch();\n        return 0;\n    }\n}\nexports.ObjectProxy = ObjectProxy;\nclass ObjectServant {\n    poa_;\n    object_id_;\n    activation_time_;\n    ref_cnt_;\n    get poa() { return this.poa_; }\n    get oid() { return this.object_id_; }\n    add_ref() {\n        let cnt = ++this.ref_cnt_;\n        //if (cnt == 1 && static_cast<impl::PoaImpl*>(poa())->pl_lifespan == Policy_Lifespan::Transient) {\n        //\tstd::lock_guard<std::mutex> lk(impl::g_orb->new_activated_objects_mut_);\n        //\tauto& list = impl::g_orb->new_activated_objects_;\n        //\tlist.erase(std::find(begin(list), end(list), this));\n        //}\n        return cnt;\n    }\n    constructor() {\n        this.ref_cnt_ = 0;\n    }\n}\nexports.ObjectServant = ObjectServant;\n;\nfunction make_simple_answer(buf, message_id) {\n    buf.consume(buf.size);\n    buf.prepare(header_size);\n    buf.write_len(header_size - 4);\n    buf.write_msg_id(message_id);\n    buf.write_msg_type(1 /* Answer */);\n    buf.commit(header_size);\n}\nexports.make_simple_answer = make_simple_answer;\nclass ReferenceList {\n    refs;\n    add_ref(obj) {\n        this.refs.push({\n            object_id: { poa_idx: obj.poa.index, object_id: obj.oid },\n            obj: obj\n        });\n        obj.add_ref();\n    }\n    // false - reference not exist\n    remove_ref(poa_idx, oid) {\n        return false;\n    }\n    constructor() {\n        this.refs = new Array();\n    }\n}\nexports.ReferenceList = ReferenceList;\n;\n//  0 - Success\n//  1 - exception\n// -1 - not handled\nfunction handle_standart_reply(buf) {\n    //if (buf.size < 8) throw new ExceptionCommFailure();\n    switch (buf.read_msg_id()) {\n        case 4 /* Success */:\n            return 0;\n        case 5 /* Exception */:\n            return 1;\n        case 7 /* Error_ObjectNotExist */:\n            throw new nprpc_base_1.ExceptionObjectNotExist();\n        case 8 /* Error_CommFailure */:\n            throw new nprpc_base_1.ExceptionCommFailure();\n        case 9 /* Error_UnknownFunctionIdx */:\n            throw new nprpc_base_1.ExceptionUnknownFunctionIndex();\n        case 10 /* Error_UnknownMessageId */:\n            throw new nprpc_base_1.ExceptionUnknownMessageId();\n        default:\n            return -1;\n    }\n}\nexports.handle_standart_reply = handle_standart_reply;\nfunction oid_create_from_flat(o) {\n    return {\n        object_id: o.object_id,\n        ip4: o.ip4,\n        port: o.port,\n        websocket_port: o.websocket_port,\n        poa_idx: o.poa_idx,\n        flags: o.flags,\n        class_id: o.class_id\n    };\n}\nexports.oid_create_from_flat = oid_create_from_flat;\nfunction narrow(from, to) {\n    if (from.data.class_id !== to.servant_t._get_class())\n        return null;\n    let obj = new to();\n    obj.data = from.data;\n    obj.local_ref_cnt_ = from.local_ref_cnt_;\n    obj.timeout_ms_ = from.timeout_ms_;\n    from.data = null;\n    from.local_ref_cnt_ = 0;\n    return obj;\n}\nexports.narrow = narrow;\nconst invalid_object_id = 0xffffffffffffffffn;\nconst localhost_ip4 = 0x7F000001;\nfunction create_object_from_flat(oid, remote_ip) {\n    if (oid.object_id == invalid_object_id)\n        return null;\n    let obj = new ObjectProxy();\n    obj.data.object_id = oid.object_id;\n    if (remote_ip == localhost_ip4) {\n        // could be the object on the same machine or from any other location\n        obj.data.ip4 = oid.ip4;\n    }\n    else {\n        if (oid.ip4 == localhost_ip4) {\n            // remote object has localhost ip converting to endpoint ip\n            obj.data.ip4 = remote_ip;\n        }\n        else {\n            // remote object with ip != localhost\n            obj.data.ip4 = oid.ip4;\n        }\n    }\n    obj.data.port = oid.port;\n    obj.data.websocket_port = oid.websocket_port;\n    obj.data.poa_idx = oid.poa_idx;\n    obj.data.flags = oid.flags;\n    obj.data.class_id = oid.class_id;\n    return obj;\n}\nexports.create_object_from_flat = create_object_from_flat;\nfunction init() {\n    exports.rpc = new Rpc();\n    return exports.rpc;\n}\nexports.init = init;\nfunction set_debug_level(debug_level) {\n    g_debug_level = debug_level;\n}\nexports.set_debug_level = set_debug_level;\n\n\n//# sourceURL=webpack://nprpc_runtime/./src/nprpc.ts?");

    /***/ }),

    /***/ "./src/nprpc_base.ts":
    /*!***************************!*\
      !*** ./src/nprpc_base.ts ***!
      \***************************/
    /***/ (function(__unused_webpack_module, exports, __webpack_require__) {

    eval("\nvar __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {\n    if (k2 === undefined) k2 = k;\n    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });\n}) : (function(o, m, k, k2) {\n    if (k2 === undefined) k2 = k;\n    o[k2] = m[k];\n}));\nvar __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {\n    Object.defineProperty(o, \"default\", { enumerable: true, value: v });\n}) : function(o, v) {\n    o[\"default\"] = v;\n});\nvar __importStar = (this && this.__importStar) || function (mod) {\n    if (mod && mod.__esModule) return mod;\n    var result = {};\n    if (mod != null) for (var k in mod) if (k !== \"default\" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);\n    __setModuleDefault(result, mod);\n    return result;\n};\nObject.defineProperty(exports, \"__esModule\", ({ value: true }));\nexports.impl = exports.detail = exports.ExceptionUnknownMessageId = exports.ExceptionUnknownFunctionIndex = exports.ExceptionObjectNotExist = exports.ExceptionTimeout = exports.Flat_nprpc_base = exports.ExceptionCommFailure = void 0;\nconst NPRPC = __importStar(__webpack_require__(/*! ./nprpc */ \"./src/nprpc.ts\"));\nclass ExceptionCommFailure extends NPRPC.Exception {\n    constructor() { super(\"ExceptionCommFailure\"); }\n}\nexports.ExceptionCommFailure = ExceptionCommFailure;\nvar Flat_nprpc_base;\n(function (Flat_nprpc_base) {\n    class ExceptionCommFailure_Direct extends NPRPC.Flat.Flat {\n        get __ex_id() { return this.buffer.dv.getUint32(this.offset + 0, true); }\n        set __ex_id(value) { this.buffer.dv.setUint32(this.offset + 0, value, true); }\n    }\n    Flat_nprpc_base.ExceptionCommFailure_Direct = ExceptionCommFailure_Direct;\n})(Flat_nprpc_base = exports.Flat_nprpc_base || (exports.Flat_nprpc_base = {})); // namespace Flat \nclass ExceptionTimeout extends NPRPC.Exception {\n    constructor() { super(\"ExceptionTimeout\"); }\n}\nexports.ExceptionTimeout = ExceptionTimeout;\n(function (Flat_nprpc_base) {\n    class ExceptionTimeout_Direct extends NPRPC.Flat.Flat {\n        get __ex_id() { return this.buffer.dv.getUint32(this.offset + 0, true); }\n        set __ex_id(value) { this.buffer.dv.setUint32(this.offset + 0, value, true); }\n    }\n    Flat_nprpc_base.ExceptionTimeout_Direct = ExceptionTimeout_Direct;\n})(Flat_nprpc_base = exports.Flat_nprpc_base || (exports.Flat_nprpc_base = {})); // namespace Flat \nclass ExceptionObjectNotExist extends NPRPC.Exception {\n    constructor() { super(\"ExceptionObjectNotExist\"); }\n}\nexports.ExceptionObjectNotExist = ExceptionObjectNotExist;\n(function (Flat_nprpc_base) {\n    class ExceptionObjectNotExist_Direct extends NPRPC.Flat.Flat {\n        get __ex_id() { return this.buffer.dv.getUint32(this.offset + 0, true); }\n        set __ex_id(value) { this.buffer.dv.setUint32(this.offset + 0, value, true); }\n    }\n    Flat_nprpc_base.ExceptionObjectNotExist_Direct = ExceptionObjectNotExist_Direct;\n})(Flat_nprpc_base = exports.Flat_nprpc_base || (exports.Flat_nprpc_base = {})); // namespace Flat \nclass ExceptionUnknownFunctionIndex extends NPRPC.Exception {\n    constructor() { super(\"ExceptionUnknownFunctionIndex\"); }\n}\nexports.ExceptionUnknownFunctionIndex = ExceptionUnknownFunctionIndex;\n(function (Flat_nprpc_base) {\n    class ExceptionUnknownFunctionIndex_Direct extends NPRPC.Flat.Flat {\n        get __ex_id() { return this.buffer.dv.getUint32(this.offset + 0, true); }\n        set __ex_id(value) { this.buffer.dv.setUint32(this.offset + 0, value, true); }\n    }\n    Flat_nprpc_base.ExceptionUnknownFunctionIndex_Direct = ExceptionUnknownFunctionIndex_Direct;\n})(Flat_nprpc_base = exports.Flat_nprpc_base || (exports.Flat_nprpc_base = {})); // namespace Flat \nclass ExceptionUnknownMessageId extends NPRPC.Exception {\n    constructor() { super(\"ExceptionUnknownMessageId\"); }\n}\nexports.ExceptionUnknownMessageId = ExceptionUnknownMessageId;\n(function (Flat_nprpc_base) {\n    class ExceptionUnknownMessageId_Direct extends NPRPC.Flat.Flat {\n        get __ex_id() { return this.buffer.dv.getUint32(this.offset + 0, true); }\n        set __ex_id(value) { this.buffer.dv.setUint32(this.offset + 0, value, true); }\n    }\n    Flat_nprpc_base.ExceptionUnknownMessageId_Direct = ExceptionUnknownMessageId_Direct;\n})(Flat_nprpc_base = exports.Flat_nprpc_base || (exports.Flat_nprpc_base = {})); // namespace Flat \nvar detail;\n(function (detail) {\n    let Flat_nprpc_base;\n    (function (Flat_nprpc_base) {\n        class ObjectIdLocal_Direct extends NPRPC.Flat.Flat {\n            get poa_idx() { return this.buffer.dv.getUint16(this.offset + 0, true); }\n            set poa_idx(value) { this.buffer.dv.setUint16(this.offset + 0, value, true); }\n            get object_id() { return this.buffer.dv.getBigUint64(this.offset + 8, true); }\n            set object_id(value) { this.buffer.dv.setBigUint64(this.offset + 8, value, true); }\n        }\n        Flat_nprpc_base.ObjectIdLocal_Direct = ObjectIdLocal_Direct;\n    })(Flat_nprpc_base = detail.Flat_nprpc_base || (detail.Flat_nprpc_base = {})); // namespace Flat \n    (function (Flat_nprpc_base) {\n        class ObjectId_Direct extends NPRPC.Flat.Flat {\n            get object_id() { return this.buffer.dv.getBigUint64(this.offset + 0, true); }\n            set object_id(value) { this.buffer.dv.setBigUint64(this.offset + 0, value, true); }\n            get ip4() { return this.buffer.dv.getUint32(this.offset + 8, true); }\n            set ip4(value) { this.buffer.dv.setUint32(this.offset + 8, value, true); }\n            get port() { return this.buffer.dv.getUint16(this.offset + 12, true); }\n            set port(value) { this.buffer.dv.setUint16(this.offset + 12, value, true); }\n            get websocket_port() { return this.buffer.dv.getUint16(this.offset + 14, true); }\n            set websocket_port(value) { this.buffer.dv.setUint16(this.offset + 14, value, true); }\n            get poa_idx() { return this.buffer.dv.getUint16(this.offset + 16, true); }\n            set poa_idx(value) { this.buffer.dv.setUint16(this.offset + 16, value, true); }\n            get flags() { return this.buffer.dv.getUint32(this.offset + 20, true); }\n            set flags(value) { this.buffer.dv.setUint32(this.offset + 20, value, true); }\n            get class_id() {\n                let enc = new TextDecoder(\"utf-8\");\n                let v_begin = this.offset + 24;\n                let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);\n                let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));\n                return enc.decode(bn);\n            }\n            set class_id(str) {\n                let enc = new TextEncoder();\n                let bytes = enc.encode(str);\n                let len = bytes.length;\n                let offset = NPRPC.Flat._alloc(this.buffer, this.offset + 24, len, 1, 1);\n                new Uint8Array(this.buffer.array_buffer, offset).set(bytes);\n            }\n        }\n        Flat_nprpc_base.ObjectId_Direct = ObjectId_Direct;\n    })(Flat_nprpc_base = detail.Flat_nprpc_base || (detail.Flat_nprpc_base = {})); // namespace Flat \n})(detail = exports.detail || (exports.detail = {})); // namespace detail\nvar impl;\n(function (impl) {\n    let Flat_nprpc_base;\n    (function (Flat_nprpc_base) {\n        class Header_Direct extends NPRPC.Flat.Flat {\n            get size() { return this.buffer.dv.getUint32(this.offset + 0, true); }\n            set size(value) { this.buffer.dv.setUint32(this.offset + 0, value, true); }\n            get msg_id() { return this.buffer.dv.getUint32(this.offset + 4, true); }\n            set msg_id(value) { this.buffer.dv.setUint32(this.offset + 4, value, true); }\n            get msg_type() { return this.buffer.dv.getUint32(this.offset + 8, true); }\n            set msg_type(value) { this.buffer.dv.setUint32(this.offset + 8, value, true); }\n            get reserved() { return this.buffer.dv.getUint32(this.offset + 12, true); }\n            set reserved(value) { this.buffer.dv.setUint32(this.offset + 12, value, true); }\n        }\n        Flat_nprpc_base.Header_Direct = Header_Direct;\n    })(Flat_nprpc_base = impl.Flat_nprpc_base || (impl.Flat_nprpc_base = {})); // namespace Flat \n    (function (Flat_nprpc_base) {\n        class CallHeader_Direct extends NPRPC.Flat.Flat {\n            get poa_idx() { return this.buffer.dv.getUint16(this.offset + 0, true); }\n            set poa_idx(value) { this.buffer.dv.setUint16(this.offset + 0, value, true); }\n            get interface_idx() { return this.buffer.dv.getUint8(this.offset + 2); }\n            set interface_idx(value) { this.buffer.dv.setUint8(this.offset + 2, value); }\n            get function_idx() { return this.buffer.dv.getUint8(this.offset + 3); }\n            set function_idx(value) { this.buffer.dv.setUint8(this.offset + 3, value); }\n            get object_id() { return this.buffer.dv.getBigUint64(this.offset + 8, true); }\n            set object_id(value) { this.buffer.dv.setBigUint64(this.offset + 8, value, true); }\n        }\n        Flat_nprpc_base.CallHeader_Direct = CallHeader_Direct;\n    })(Flat_nprpc_base = impl.Flat_nprpc_base || (impl.Flat_nprpc_base = {})); // namespace Flat \n})(impl = exports.impl || (exports.impl = {})); // namespace impl\nfunction nprpc_base_throw_exception(buf) {\n    switch (buf.read_exception_number()) {\n        case 0:\n            {\n                let ex_flat = new Flat_nprpc_base.ExceptionCommFailure_Direct(buf, 16);\n                let ex = new ExceptionCommFailure();\n                throw ex;\n            }\n        case 1:\n            {\n                let ex_flat = new Flat_nprpc_base.ExceptionTimeout_Direct(buf, 16);\n                let ex = new ExceptionTimeout();\n                throw ex;\n            }\n        case 2:\n            {\n                let ex_flat = new Flat_nprpc_base.ExceptionObjectNotExist_Direct(buf, 16);\n                let ex = new ExceptionObjectNotExist();\n                throw ex;\n            }\n        case 3:\n            {\n                let ex_flat = new Flat_nprpc_base.ExceptionUnknownFunctionIndex_Direct(buf, 16);\n                let ex = new ExceptionUnknownFunctionIndex();\n                throw ex;\n            }\n        case 4:\n            {\n                let ex_flat = new Flat_nprpc_base.ExceptionUnknownMessageId_Direct(buf, 16);\n                let ex = new ExceptionUnknownMessageId();\n                throw ex;\n            }\n        default:\n            throw \"unknown rpc exception\";\n    }\n}\n\n\n//# sourceURL=webpack://nprpc_runtime/./src/nprpc_base.ts?");

    /***/ }),

    /***/ "./src/nprpc_nameserver.ts":
    /*!*********************************!*\
      !*** ./src/nprpc_nameserver.ts ***!
      \*********************************/
    /***/ (function(__unused_webpack_module, exports, __webpack_require__) {

    eval("\nvar __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {\n    if (k2 === undefined) k2 = k;\n    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });\n}) : (function(o, m, k, k2) {\n    if (k2 === undefined) k2 = k;\n    o[k2] = m[k];\n}));\nvar __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {\n    Object.defineProperty(o, \"default\", { enumerable: true, value: v });\n}) : function(o, v) {\n    o[\"default\"] = v;\n});\nvar __importStar = (this && this.__importStar) || function (mod) {\n    if (mod && mod.__esModule) return mod;\n    var result = {};\n    if (mod != null) for (var k in mod) if (k !== \"default\" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);\n    __setModuleDefault(result, mod);\n    return result;\n};\nObject.defineProperty(exports, \"__esModule\", ({ value: true }));\nexports.Flat_nprpc_nameserver = exports._INameserver_Servant = exports.Nameserver = void 0;\nconst NPRPC = __importStar(__webpack_require__(/*! ./nprpc */ \"./src/nprpc.ts\"));\nclass Nameserver extends NPRPC.ObjectProxy {\n    static get servant_t() {\n        return _INameserver_Servant;\n    }\n    async Bind(obj, name) {\n        let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);\n        let buf = NPRPC.FlatBuffer.create();\n        buf.prepare(200);\n        buf.commit(72);\n        buf.write_msg_id(0 /* FunctionCall */);\n        buf.write_msg_type(0 /* Request */);\n        let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);\n        __ch.object_id = this.data.object_id;\n        __ch.poa_idx = this.data.poa_idx;\n        __ch.interface_idx = interface_idx;\n        __ch.function_idx = 0;\n        let _ = new Flat_nprpc_nameserver.nprpc_nameserver_M1_Direct(buf, 32);\n        _._1.object_id = obj.object_id;\n        _._1.ip4 = obj.ip4;\n        _._1.port = obj.port;\n        _._1.websocket_port = obj.websocket_port;\n        _._1.poa_idx = obj.poa_idx;\n        _._1.flags = obj.flags;\n        _._1.class_id = obj.class_id;\n        _._2 = name;\n        buf.write_len(buf.size - 4);\n        await NPRPC.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);\n        let std_reply = NPRPC.handle_standart_reply(buf);\n        if (std_reply != 0) {\n            console.log(\"received an unusual reply for function with no output arguments\");\n        }\n    }\n    async Resolve(name, obj) {\n        let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);\n        let buf = NPRPC.FlatBuffer.create();\n        buf.prepare(168);\n        buf.commit(40);\n        buf.write_msg_id(0 /* FunctionCall */);\n        buf.write_msg_type(0 /* Request */);\n        let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);\n        __ch.object_id = this.data.object_id;\n        __ch.poa_idx = this.data.poa_idx;\n        __ch.interface_idx = interface_idx;\n        __ch.function_idx = 1;\n        let _ = new Flat_nprpc_nameserver.nprpc_nameserver_M2_Direct(buf, 32);\n        _._1 = name;\n        buf.write_len(buf.size - 4);\n        await NPRPC.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);\n        let std_reply = NPRPC.handle_standart_reply(buf);\n        if (std_reply != -1) {\n            console.log(\"received an unusual reply for function with output arguments\");\n            throw new NPRPC.Exception(\"Unknown Error\");\n        }\n        let out = new Flat_nprpc_nameserver.nprpc_nameserver_M3_Direct(buf, 16);\n        obj.value = NPRPC.create_object_from_flat(out._2, this.data.ip4);\n        let __ret_value /*boolean*/;\n        __ret_value = out._1;\n        return __ret_value;\n    }\n}\nexports.Nameserver = Nameserver;\n;\nclass _INameserver_Servant extends NPRPC.ObjectServant {\n    static _get_class() { return \"nprpc_nameserver/nprpc.Nameserver\"; }\n    get_class = () => { return _INameserver_Servant._get_class(); };\n    dispatch = (buf, remote_endpoint, from_parent) => {\n        _INameserver_Servant._dispatch(this, buf, remote_endpoint, from_parent);\n    };\n    static _dispatch(obj, buf, remote_endpoint, from_parent) {\n        let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);\n        switch (__ch.function_idx) {\n            case 0: {\n                let ia = new Flat_nprpc_nameserver.nprpc_nameserver_M1_Direct(buf, 32);\n                obj.Bind(NPRPC.create_object_from_flat(ia._1, remote_endpoint.ip4), ia._2);\n                NPRPC.make_simple_answer(buf, 4 /* Success */);\n                break;\n            }\n            case 1: {\n                let ia = new Flat_nprpc_nameserver.nprpc_nameserver_M2_Direct(buf, 32);\n                let obuf = buf;\n                obuf.consume(obuf.size);\n                obuf.prepare(184);\n                obuf.commit(56);\n                let oa = new Flat_nprpc_nameserver.nprpc_nameserver_M3_Direct(obuf, 16);\n                let __ret_val /*boolean*/;\n                __ret_val = obj.Resolve(ia._1, oa._2);\n                oa._1 = __ret_val;\n                obuf.write_len(obuf.size - 4);\n                obuf.write_msg_id(1 /* BlockResponse */);\n                obuf.write_msg_type(1 /* Answer */);\n                break;\n            }\n            default:\n                NPRPC.make_simple_answer(buf, 9 /* Error_UnknownFunctionIdx */);\n        }\n    }\n}\nexports._INameserver_Servant = _INameserver_Servant;\nvar Flat_nprpc_nameserver;\n(function (Flat_nprpc_nameserver) {\n    class nprpc_nameserver_M1_Direct extends NPRPC.Flat.Flat {\n        get _1() { return new NPRPC.detail.Flat_nprpc_base.ObjectId_Direct(this.buffer, this.offset + 0); }\n        get _2() {\n            let enc = new TextDecoder(\"utf-8\");\n            let v_begin = this.offset + 32;\n            let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);\n            let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));\n            return enc.decode(bn);\n        }\n        set _2(str) {\n            let enc = new TextEncoder();\n            let bytes = enc.encode(str);\n            let len = bytes.length;\n            let offset = NPRPC.Flat._alloc(this.buffer, this.offset + 32, len, 1, 1);\n            new Uint8Array(this.buffer.array_buffer, offset).set(bytes);\n        }\n    }\n    Flat_nprpc_nameserver.nprpc_nameserver_M1_Direct = nprpc_nameserver_M1_Direct;\n})(Flat_nprpc_nameserver = exports.Flat_nprpc_nameserver || (exports.Flat_nprpc_nameserver = {})); // namespace Flat \n(function (Flat_nprpc_nameserver) {\n    class nprpc_nameserver_M2_Direct extends NPRPC.Flat.Flat {\n        get _1() {\n            let enc = new TextDecoder(\"utf-8\");\n            let v_begin = this.offset + 0;\n            let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);\n            let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));\n            return enc.decode(bn);\n        }\n        set _1(str) {\n            let enc = new TextEncoder();\n            let bytes = enc.encode(str);\n            let len = bytes.length;\n            let offset = NPRPC.Flat._alloc(this.buffer, this.offset + 0, len, 1, 1);\n            new Uint8Array(this.buffer.array_buffer, offset).set(bytes);\n        }\n    }\n    Flat_nprpc_nameserver.nprpc_nameserver_M2_Direct = nprpc_nameserver_M2_Direct;\n})(Flat_nprpc_nameserver = exports.Flat_nprpc_nameserver || (exports.Flat_nprpc_nameserver = {})); // namespace Flat \n(function (Flat_nprpc_nameserver) {\n    class nprpc_nameserver_M3_Direct extends NPRPC.Flat.Flat {\n        get _1() { return (this.buffer.dv.getUint8(this.offset + 0) === 0x01); }\n        set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value === true ? 0x01 : 0x00); }\n        get _2() { return new NPRPC.detail.Flat_nprpc_base.ObjectId_Direct(this.buffer, this.offset + 8); }\n    }\n    Flat_nprpc_nameserver.nprpc_nameserver_M3_Direct = nprpc_nameserver_M3_Direct;\n})(Flat_nprpc_nameserver = exports.Flat_nprpc_nameserver || (exports.Flat_nprpc_nameserver = {})); // namespace Flat \n\n\n//# sourceURL=webpack://nprpc_runtime/./src/nprpc_nameserver.ts?");

    /***/ })

    /******/ 	});
    /************************************************************************/
    /******/ 	// The module cache
    /******/ 	var __webpack_module_cache__ = {};
    /******/ 	
    /******/ 	// The require function
    /******/ 	function __webpack_require__(moduleId) {
    /******/ 		// Check if module is in cache
    /******/ 		var cachedModule = __webpack_module_cache__[moduleId];
    /******/ 		if (cachedModule !== undefined) {
    /******/ 			return cachedModule.exports;
    /******/ 		}
    /******/ 		// Create a new module (and put it into the cache)
    /******/ 		var module = __webpack_module_cache__[moduleId] = {
    /******/ 			// no module.id needed
    /******/ 			// no module.loaded needed
    /******/ 			exports: {}
    /******/ 		};
    /******/ 	
    /******/ 		// Execute the module function
    /******/ 		__webpack_modules__[moduleId].call(module.exports, module, module.exports, __webpack_require__);
    /******/ 	
    /******/ 		// Return the exports of the module
    /******/ 		return module.exports;
    /******/ 	}
    /******/ 	
    /************************************************************************/
    /******/ 	
    /******/ 	// startup
    /******/ 	// Load entry module and return exports
    /******/ 	// This entry module is referenced by other modules so it can't be inlined
    /******/ 	var __webpack_exports__ = __webpack_require__("./src/index.ts");
    /******/ 	
    /******/ 	return __webpack_exports__;
    /******/ })()
    ;
    });
    });

    var Flat_npwebserver;
    (function (Flat_npwebserver) {
        class WebItem_Direct extends nprpc.Flat.Flat {
            get name() {
                let enc = new TextDecoder("utf-8");
                let v_begin = this.offset + 0;
                let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);
                let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));
                return enc.decode(bn);
            }
            set name(str) {
                let enc = new TextEncoder();
                let bytes = enc.encode(str);
                let len = bytes.length;
                let offset = nprpc.Flat._alloc(this.buffer, this.offset + 0, len, 1, 1);
                new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
            }
            get description() {
                let enc = new TextDecoder("utf-8");
                let v_begin = this.offset + 8;
                let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);
                let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));
                return enc.decode(bn);
            }
            set description(str) {
                let enc = new TextEncoder();
                let bytes = enc.encode(str);
                let len = bytes.length;
                let offset = nprpc.Flat._alloc(this.buffer, this.offset + 8, len, 1, 1);
                new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
            }
            get path() {
                let enc = new TextDecoder("utf-8");
                let v_begin = this.offset + 16;
                let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);
                let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));
                return enc.decode(bn);
            }
            set path(str) {
                let enc = new TextEncoder();
                let bytes = enc.encode(str);
                let len = bytes.length;
                let offset = nprpc.Flat._alloc(this.buffer, this.offset + 16, len, 1, 1);
                new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
            }
            get dev_addr() { return this.buffer.dv.getUint8(this.offset + 24); }
            set dev_addr(value) { this.buffer.dv.setUint8(this.offset + 24, value); }
            get mem_addr() { return this.buffer.dv.getUint16(this.offset + 26, true); }
            set mem_addr(value) { this.buffer.dv.setUint16(this.offset + 26, value, true); }
            get type() { return this.buffer.dv.getUint32(this.offset + 28, true); }
            set type(value) { this.buffer.dv.setUint32(this.offset + 28, value, true); }
        }
        Flat_npwebserver.WebItem_Direct = WebItem_Direct;
    })(Flat_npwebserver || (Flat_npwebserver = {})); // namespace Flat 
    (function (Flat_npwebserver) {
        class WebCategory_Direct extends nprpc.Flat.Flat {
            get name() {
                let enc = new TextDecoder("utf-8");
                let v_begin = this.offset + 0;
                let data_offset = v_begin + this.buffer.dv.getUint32(v_begin, true);
                let bn = this.buffer.array_buffer.slice(data_offset, data_offset + this.buffer.dv.getUint32(v_begin + 4, true));
                return enc.decode(bn);
            }
            set name(str) {
                let enc = new TextEncoder();
                let bytes = enc.encode(str);
                let len = bytes.length;
                let offset = nprpc.Flat._alloc(this.buffer, this.offset + 0, len, 1, 1);
                new Uint8Array(this.buffer.array_buffer, offset).set(bytes);
            }
            items(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 8, elements_size, 32, 4);
            }
            items_vd() { return new nprpc.Flat.Vector_Direct2(this.buffer, this.offset + 8, 32, Flat_npwebserver.WebItem_Direct); }
        }
        Flat_npwebserver.WebCategory_Direct = WebCategory_Direct;
    })(Flat_npwebserver || (Flat_npwebserver = {})); // namespace Flat 
    class WebServer extends nprpc.ObjectProxy {
        static get servant_t() {
            return _IWebServer_Servant;
        }
        async get_web_categories(cats) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(32);
            buf.commit(32);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 0;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_npwebserver.npwebserver_M1_Direct(buf, 16);
            {
                let vv = out._1_vd(), index_0 = 0;
                cats.length = vv.elements_size;
                for (let e of vv) {
                    cats[index_0] = new Object();
                    cats[index_0] = new Object();
                    cats[index_0].name = e.name;
                    {
                        let vv = e.items_vd(), index_1 = 0;
                        cats[index_0].items = new Array(vv.elements_size);
                        for (let e of vv) {
                            cats[index_0].items[index_1] = new Object();
                            cats[index_0].items[index_1] = new Object();
                            cats[index_0].items[index_1].name = e.name;
                            cats[index_0].items[index_1].description = e.description;
                            cats[index_0].items[index_1].path = e.path;
                            cats[index_0].items[index_1].dev_addr = e.dev_addr;
                            cats[index_0].items[index_1].mem_addr = e.mem_addr;
                            cats[index_0].items[index_1].type = e.type;
                            ++index_1;
                        }
                    }
                    ++index_0;
                }
            }
        }
    }
    class _IWebServer_Servant extends nprpc.ObjectServant {
        static _get_class() { return "npwebserver/npwebserver.WebServer"; }
        get_class = () => { return _IWebServer_Servant._get_class(); };
        dispatch = (buf, remote_endpoint, from_parent) => {
            _IWebServer_Servant._dispatch(this, buf, remote_endpoint, from_parent);
        };
        static _dispatch(obj, buf, remote_endpoint, from_parent) {
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            switch (__ch.function_idx) {
                case 0: {
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(152);
                    obuf.commit(24);
                    let oa = new Flat_npwebserver.npwebserver_M1_Direct(obuf, 16);
                    obj.get_web_categories(oa._1_vd);
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                default:
                    nprpc.make_simple_answer(buf, 9 /* Error_UnknownFunctionIdx */);
            }
        }
    }
    (function (Flat_npwebserver) {
        class npwebserver_M1_Direct extends nprpc.Flat.Flat {
            _1(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 0, elements_size, 16, 4);
            }
            _1_vd() { return new nprpc.Flat.Vector_Direct2(this.buffer, this.offset + 0, 16, Flat_npwebserver.WebCategory_Direct); }
        }
        Flat_npwebserver.npwebserver_M1_Direct = npwebserver_M1_Direct;
    })(Flat_npwebserver || (Flat_npwebserver = {})); // namespace Flat

    class NPNetCommunicationError extends nprpc.Exception {
        code;
        constructor(code /*i32*/) {
            super("NPNetCommunicationError");
            this.code = code;
        }
    }
    var Flat_server;
    (function (Flat_server) {
        class NPNetCommunicationError_Direct extends nprpc.Flat.Flat {
            get __ex_id() { return this.buffer.dv.getUint32(this.offset + 0, true); }
            set __ex_id(value) { this.buffer.dv.setUint32(this.offset + 0, value, true); }
            get code() { return this.buffer.dv.getInt32(this.offset + 4, true); }
            set code(value) { this.buffer.dv.setInt32(this.offset + 4, value, true); }
        }
        Flat_server.NPNetCommunicationError_Direct = NPNetCommunicationError_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_value_Direct extends nprpc.Flat.Flat {
            get h() { return this.buffer.dv.getBigUint64(this.offset + 0, true); }
            set h(value) { this.buffer.dv.setBigUint64(this.offset + 0, value, true); }
            get s() { return this.buffer.dv.getUint32(this.offset + 8, true); }
            set s(value) { this.buffer.dv.setUint32(this.offset + 8, value, true); }
            data_vd() { return new nprpc.Flat.Array_Direct1_u8(this.buffer, this.offset + 12, 8); }
        }
        Flat_server.server_value_Direct = server_value_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class RawDataDef_Direct extends nprpc.Flat.Flat {
            get dev_addr() { return this.buffer.dv.getUint8(this.offset + 0); }
            set dev_addr(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get address() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set address(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get size() { return this.buffer.dv.getUint8(this.offset + 4); }
            set size(value) { this.buffer.dv.setUint8(this.offset + 4, value); }
        }
        Flat_server.RawDataDef_Direct = RawDataDef_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class DataDef_Direct extends nprpc.Flat.Flat {
            get dev_addr() { return this.buffer.dv.getUint8(this.offset + 0); }
            set dev_addr(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get mem_addr() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set mem_addr(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get type() { return this.buffer.dv.getUint32(this.offset + 4, true); }
            set type(value) { this.buffer.dv.setUint32(this.offset + 4, value, true); }
        }
        Flat_server.DataDef_Direct = DataDef_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    class Pingable extends nprpc.ObjectProxy {
        static get servant_t() {
            return _IPingable_Servant;
        }
        async Ping() {
            let interface_idx = (arguments.length == 0 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(32);
            buf.commit(32);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 0;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
    }
    class _IPingable_Servant extends nprpc.ObjectServant {
        static _get_class() { return "server/nps.Pingable"; }
        get_class = () => { return _IPingable_Servant._get_class(); };
        dispatch = (buf, remote_endpoint, from_parent) => {
            _IPingable_Servant._dispatch(this, buf, remote_endpoint, from_parent);
        };
        static _dispatch(obj, buf, remote_endpoint, from_parent) {
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            switch (__ch.function_idx) {
                case 0: {
                    obj.Ping();
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                default:
                    nprpc.make_simple_answer(buf, 9 /* Error_UnknownFunctionIdx */);
            }
        }
    }
    class _IDataCallBack_Servant extends nprpc.ObjectServant {
        static _get_class() { return "server/nps.DataCallBack"; }
        get_class = () => { return _IDataCallBack_Servant._get_class(); };
        dispatch = (buf, remote_endpoint, from_parent) => {
            _IDataCallBack_Servant._dispatch(this, buf, remote_endpoint, from_parent);
        };
        static _dispatch(obj, buf, remote_endpoint, from_parent) {
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            if (from_parent == false) {
                switch (__ch.interface_idx) {
                    case 0:
                        break;
                    case 1:
                        _IPingable_Servant._dispatch(obj, buf, remote_endpoint, true);
                        return;
                    default:
                        throw "unknown interface";
                }
            }
            switch (__ch.function_idx) {
                case 0: {
                    let ia = new Flat_server.server_M1_Direct(buf, 32);
                    obj.OnDataChanged(ia._1_vd());
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                default:
                    nprpc.make_simple_answer(buf, 9 /* Error_UnknownFunctionIdx */);
            }
        }
    }
    class ItemManager extends nprpc.ObjectProxy {
        static get servant_t() {
            return _IItemManager_Servant;
        }
        // Pingable
        async Ping() {
            Pingable.prototype.Ping.bind(this, 1)();
        }
        async Activate(client) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(192);
            buf.commit(64);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 0;
            let _ = new Flat_server.server_M2_Direct(buf, 32);
            _._1.object_id = client.object_id;
            _._1.ip4 = client.ip4;
            _._1.port = client.port;
            _._1.websocket_port = client.websocket_port;
            _._1.poa_idx = client.poa_idx;
            _._1.flags = client.flags;
            _._1.class_id = client.class_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Advise(a, h) {
            let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(168);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 1;
            let _ = new Flat_server.server_M3_Direct(buf, 32);
            _._1(a.length);
            {
                let vv = _._1_vd(), index = 0;
                for (let e of vv) {
                    e.dev_addr = a[index].dev_addr;
                    e.mem_addr = a[index].mem_addr;
                    e.type = a[index].type;
                    ++index;
                }
            }
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M4_Direct(buf, 16);
            {
                let vv = out._1_vd(), index_0 = 0;
                h.length = vv.elements_size;
                for (let e of vv) {
                    h[index_0] = e;
                    ++index_0;
                }
            }
        }
        async UnAdvise(a) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(168);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 2;
            let _ = new Flat_server.server_M4_Direct(buf, 32);
            _._1(a.length);
            _._1_vd().copy_from_ts_array(a);
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
    }
    class _IItemManager_Servant extends nprpc.ObjectServant {
        static _get_class() { return "server/nps.ItemManager"; }
        get_class = () => { return _IItemManager_Servant._get_class(); };
        dispatch = (buf, remote_endpoint, from_parent) => {
            _IItemManager_Servant._dispatch(this, buf, remote_endpoint, from_parent);
        };
        static _dispatch(obj, buf, remote_endpoint, from_parent) {
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            if (from_parent == false) {
                switch (__ch.interface_idx) {
                    case 0:
                        break;
                    case 1:
                        _IPingable_Servant._dispatch(obj, buf, remote_endpoint, true);
                        return;
                    default:
                        throw "unknown interface";
                }
            }
            switch (__ch.function_idx) {
                case 0: {
                    let ia = new Flat_server.server_M2_Direct(buf, 32);
                    obj.Activate(nprpc.create_object_from_flat(ia._1, remote_endpoint.ip4));
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 1: {
                    let ia = new Flat_server.server_M3_Direct(buf, 32);
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(152);
                    obuf.commit(24);
                    let oa = new Flat_server.server_M4_Direct(obuf, 16);
                    try {
                        obj.Advise(ia._1_vd(), oa._1_vd);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 2: {
                    let ia = new Flat_server.server_M4_Direct(buf, 32);
                    obj.UnAdvise(ia._1_vd());
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                default:
                    nprpc.make_simple_answer(buf, 9 /* Error_UnknownFunctionIdx */);
            }
        }
    }
    class Server extends nprpc.ObjectProxy {
        static get servant_t() {
            return _IServer_Servant;
        }
        // Pingable
        async Ping() {
            Pingable.prototype.Ping.bind(this, 1)();
        }
        async GetNetworkStatus(network_status) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(32);
            buf.commit(32);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 0;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M5_Direct(buf, 16);
            {
                let vv = out._1_vd(), index_1 = 0;
                network_status.length = vv.elements_size;
                for (let e of vv) {
                    network_status[index_1] = e;
                    ++index_1;
                }
            }
        }
        async CreateItemManager(im) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(32);
            buf.commit(32);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 1;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M2_Direct(buf, 16);
            im.value = nprpc.create_object_from_flat(out._1, this.data.ip4);
        }
        async SendRawData(data) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(168);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 2;
            let _ = new Flat_server.server_M5_Direct(buf, 32);
            _._1(data.length);
            _._1_vd().copy_from_ts_array(data);
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Write_1(dev_addr, mem_addr, bit, value) {
            let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(38);
            buf.commit(38);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 3;
            let _ = new Flat_server.server_M6_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3 = bit;
            _._4 = value;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Write_q1(dev_addr, mem_addr, bit, value, qvalue) {
            let interface_idx = (arguments.length == 5 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 4;
            let _ = new Flat_server.server_M7_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3 = bit;
            _._4 = value;
            _._5 = qvalue;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Write_8(dev_addr, mem_addr, value) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(38);
            buf.commit(38);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 5;
            let _ = new Flat_server.server_M8_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3 = value;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Write_q8(dev_addr, mem_addr, value, qvalue) {
            let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(38);
            buf.commit(38);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 6;
            let _ = new Flat_server.server_M6_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3 = value;
            _._4 = qvalue;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Write_16(dev_addr, mem_addr, value) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(38);
            buf.commit(38);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 7;
            let _ = new Flat_server.server_M9_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3 = value;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Write_q16(dev_addr, mem_addr, value, qvalue) {
            let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 8;
            let _ = new Flat_server.server_M10_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3 = value;
            _._4 = qvalue;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Write_32(dev_addr, mem_addr, value) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 9;
            let _ = new Flat_server.server_M11_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3 = value;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Write_q32(dev_addr, mem_addr, value, qvalue) {
            let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(44);
            buf.commit(44);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 10;
            let _ = new Flat_server.server_M12_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3 = value;
            _._4 = qvalue;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async WriteBlock(dev_addr, mem_addr, data) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(172);
            buf.commit(44);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 11;
            let _ = new Flat_server.server_M13_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3(data.length);
            _._3_vd().copy_from_ts_array(data);
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async ReadByte(dev_addr, addr, value) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(36);
            buf.commit(36);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 12;
            let _ = new Flat_server.server_M14_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = addr;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M15_Direct(buf, 16);
            value.value = out._1;
        }
        async ReadBlock(dev_addr, addr, len, data) {
            let interface_idx = (arguments.length == 4 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(38);
            buf.commit(38);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 13;
            let _ = new Flat_server.server_M8_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = addr;
            _._3 = len;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M5_Direct(buf, 16);
            {
                let vv = out._1_vd(), index_2 = 0;
                data.length = vv.elements_size;
                for (let e of vv) {
                    data[index_2] = e;
                    ++index_2;
                }
            }
        }
        async AVR_StopAlgorithm(dev_addr, alg_addr) {
            let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(36);
            buf.commit(36);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 14;
            let _ = new Flat_server.server_M14_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = alg_addr;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M16_Direct(buf, 16);
            let __ret_value /*boolean*/;
            __ret_value = out._1;
            return __ret_value;
        }
        async AVR_ReinitIO(dev_addr) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(33);
            buf.commit(33);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 15;
            let _ = new Flat_server.server_M15_Direct(buf, 32);
            _._1 = dev_addr;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async AVR_SendRemoteData(dev_addr, page_num, data) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(172);
            buf.commit(44);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 16;
            let _ = new Flat_server.server_M13_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = page_num;
            _._3(data.length);
            _._3_vd().copy_from_ts_array(data);
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async AVR_SendPage(dev_addr, page_num, data) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(172);
            buf.commit(44);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 17;
            let _ = new Flat_server.server_M17_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = page_num;
            _._3(data.length);
            _._3_vd().copy_from_ts_array(data);
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async AVR_RemoveAlgorithm(dev_addr, alg_addr) {
            let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(36);
            buf.commit(36);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 18;
            let _ = new Flat_server.server_M14_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = alg_addr;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async AVR_ReplaceAlgorithm(dev_addr, old_addr, new_addr) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(38);
            buf.commit(38);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 19;
            let _ = new Flat_server.server_M9_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = old_addr;
            _._3 = new_addr;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async AVR_WriteEeprom(dev_addr, mem_addr, data) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(172);
            buf.commit(44);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 20;
            let _ = new Flat_server.server_M13_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = mem_addr;
            _._3(data.length);
            _._3_vd().copy_from_ts_array(data);
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async AVR_WriteTwiTable(dev_addr, page_num, data) {
            let interface_idx = (arguments.length == 3 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(172);
            buf.commit(44);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 21;
            let _ = new Flat_server.server_M17_Direct(buf, 32);
            _._1 = dev_addr;
            _._2 = page_num;
            _._3(data.length);
            _._3_vd().copy_from_ts_array(data);
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async AVR_V_GetFlash(device_id, data) {
            let interface_idx = (arguments.length == 2 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 22;
            let _ = new Flat_server.server_M18_Direct(buf, 32);
            _._1 = device_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M19_Direct(buf, 16);
            {
                let vv = out._1_vd(), index_3 = 0;
                data.length = vv.elements_size;
                for (let e of vv) {
                    data[index_3] = e;
                    ++index_3;
                }
            }
        }
        async AVR_V_StoreFlash(device_id) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 23;
            let _ = new Flat_server.server_M18_Direct(buf, 32);
            _._1 = device_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply == 1) {
                server_throw_exception(buf);
            }
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M16_Direct(buf, 16);
            let __ret_value /*boolean*/;
            __ret_value = out._1;
            return __ret_value;
        }
        async Notify_DeviceActivated(device_id) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 24;
            let _ = new Flat_server.server_M18_Direct(buf, 32);
            _._1 = device_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M16_Direct(buf, 16);
            let __ret_value /*boolean*/;
            __ret_value = out._1;
            return __ret_value;
        }
        async Notify_DeviceDeactivated(device_id) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 25;
            let _ = new Flat_server.server_M18_Direct(buf, 32);
            _._1 = device_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != -1) {
                console.log("received an unusual reply for function with output arguments");
                throw new nprpc.Exception("Unknown Error");
            }
            let out = new Flat_server.server_M16_Direct(buf, 16);
            let __ret_value /*boolean*/;
            __ret_value = out._1;
            return __ret_value;
        }
        async Notify_ParameterRemoved(param_id) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 26;
            let _ = new Flat_server.server_M18_Direct(buf, 32);
            _._1 = param_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async Notify_TypeOrVariableChanged(param_id) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 27;
            let _ = new Flat_server.server_M18_Direct(buf, 32);
            _._1 = param_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async History_AddParameter(param_id) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 28;
            let _ = new Flat_server.server_M18_Direct(buf, 32);
            _._1 = param_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
        async History_RemoveParameter(param_id) {
            let interface_idx = (arguments.length == 1 ? 0 : arguments[arguments.length - 1]);
            let buf = nprpc.FlatBuffer.create();
            buf.prepare(40);
            buf.commit(40);
            buf.write_msg_id(0 /* FunctionCall */);
            buf.write_msg_type(0 /* Request */);
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            __ch.object_id = this.data.object_id;
            __ch.poa_idx = this.data.poa_idx;
            __ch.interface_idx = interface_idx;
            __ch.function_idx = 29;
            let _ = new Flat_server.server_M18_Direct(buf, 32);
            _._1 = param_id;
            buf.write_len(buf.size - 4);
            await nprpc.rpc.call({ ip4: this.data.ip4, port: this.data.websocket_port }, buf, this.timeout);
            let std_reply = nprpc.handle_standart_reply(buf);
            if (std_reply != 0) {
                console.log("received an unusual reply for function with no output arguments");
            }
        }
    }
    class _IServer_Servant extends nprpc.ObjectServant {
        static _get_class() { return "server/nps.Server"; }
        get_class = () => { return _IServer_Servant._get_class(); };
        dispatch = (buf, remote_endpoint, from_parent) => {
            _IServer_Servant._dispatch(this, buf, remote_endpoint, from_parent);
        };
        static _dispatch(obj, buf, remote_endpoint, from_parent) {
            let __ch = new nprpc.impl.Flat_nprpc_base.CallHeader_Direct(buf, 16);
            if (from_parent == false) {
                switch (__ch.interface_idx) {
                    case 0:
                        break;
                    case 1:
                        _IPingable_Servant._dispatch(obj, buf, remote_endpoint, true);
                        return;
                    default:
                        throw "unknown interface";
                }
            }
            switch (__ch.function_idx) {
                case 0: {
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(152);
                    obuf.commit(24);
                    let oa = new Flat_server.server_M5_Direct(obuf, 16);
                    obj.GetNetworkStatus(oa._1_vd);
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 1: {
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(176);
                    obuf.commit(48);
                    let oa = new Flat_server.server_M2_Direct(obuf, 16);
                    obj.CreateItemManager(oa._1);
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 2: {
                    let ia = new Flat_server.server_M5_Direct(buf, 32);
                    try {
                        obj.SendRawData(ia._1_vd());
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 3: {
                    let ia = new Flat_server.server_M6_Direct(buf, 32);
                    try {
                        obj.Write_1(ia._1, ia._2, ia._3, ia._4);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 4: {
                    let ia = new Flat_server.server_M7_Direct(buf, 32);
                    try {
                        obj.Write_q1(ia._1, ia._2, ia._3, ia._4, ia._5);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 5: {
                    let ia = new Flat_server.server_M8_Direct(buf, 32);
                    try {
                        obj.Write_8(ia._1, ia._2, ia._3);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 6: {
                    let ia = new Flat_server.server_M6_Direct(buf, 32);
                    try {
                        obj.Write_q8(ia._1, ia._2, ia._3, ia._4);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 7: {
                    let ia = new Flat_server.server_M9_Direct(buf, 32);
                    try {
                        obj.Write_16(ia._1, ia._2, ia._3);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 8: {
                    let ia = new Flat_server.server_M10_Direct(buf, 32);
                    try {
                        obj.Write_q16(ia._1, ia._2, ia._3, ia._4);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 9: {
                    let ia = new Flat_server.server_M11_Direct(buf, 32);
                    try {
                        obj.Write_32(ia._1, ia._2, ia._3);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 10: {
                    let ia = new Flat_server.server_M12_Direct(buf, 32);
                    try {
                        obj.Write_q32(ia._1, ia._2, ia._3, ia._4);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 11: {
                    let ia = new Flat_server.server_M13_Direct(buf, 32);
                    try {
                        obj.WriteBlock(ia._1, ia._2, ia._3_vd());
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 12: {
                    let _out_1 /*u8*/;
                    let ia = new Flat_server.server_M14_Direct(buf, 32);
                    try {
                        obj.ReadByte(ia._1, ia._2, _out_1);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(17);
                    obuf.commit(17);
                    let oa = new Flat_server.server_M15_Direct(obuf, 16);
                    oa._1 = _out_1;
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 13: {
                    let ia = new Flat_server.server_M8_Direct(buf, 32);
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(152);
                    obuf.commit(24);
                    let oa = new Flat_server.server_M5_Direct(obuf, 16);
                    try {
                        obj.ReadBlock(ia._1, ia._2, ia._3, oa._1_vd);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 14: {
                    let ia = new Flat_server.server_M14_Direct(buf, 32);
                    let __ret_val /*boolean*/;
                    try {
                        __ret_val = obj.AVR_StopAlgorithm(ia._1, ia._2);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(17);
                    obuf.commit(17);
                    let oa = new Flat_server.server_M16_Direct(obuf, 16);
                    oa._1 = __ret_val;
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 15: {
                    let ia = new Flat_server.server_M15_Direct(buf, 32);
                    try {
                        obj.AVR_ReinitIO(ia._1);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 16: {
                    let ia = new Flat_server.server_M13_Direct(buf, 32);
                    try {
                        obj.AVR_SendRemoteData(ia._1, ia._2, ia._3_vd());
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 17: {
                    let ia = new Flat_server.server_M17_Direct(buf, 32);
                    try {
                        obj.AVR_SendPage(ia._1, ia._2, ia._3_vd());
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 18: {
                    let ia = new Flat_server.server_M14_Direct(buf, 32);
                    try {
                        obj.AVR_RemoveAlgorithm(ia._1, ia._2);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 19: {
                    let ia = new Flat_server.server_M9_Direct(buf, 32);
                    try {
                        obj.AVR_ReplaceAlgorithm(ia._1, ia._2, ia._3);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 20: {
                    let ia = new Flat_server.server_M13_Direct(buf, 32);
                    try {
                        obj.AVR_WriteEeprom(ia._1, ia._2, ia._3_vd());
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 21: {
                    let ia = new Flat_server.server_M17_Direct(buf, 32);
                    try {
                        obj.AVR_WriteTwiTable(ia._1, ia._2, ia._3_vd());
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 22: {
                    let ia = new Flat_server.server_M18_Direct(buf, 32);
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(152);
                    obuf.commit(24);
                    let oa = new Flat_server.server_M19_Direct(obuf, 16);
                    try {
                        obj.AVR_V_GetFlash(ia._1, oa._1_vd);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 23: {
                    let ia = new Flat_server.server_M18_Direct(buf, 32);
                    let __ret_val /*boolean*/;
                    try {
                        __ret_val = obj.AVR_V_StoreFlash(ia._1);
                    }
                    catch (e) {
                        let obuf = buf;
                        obuf.consume(obuf.size);
                        obuf.prepare(24);
                        obuf.commit(24);
                        let oa = new Flat_server.NPNetCommunicationError_Direct(obuf, 16);
                        oa.__ex_id = 0;
                        oa.code = e.code;
                        obuf.write_len(obuf.size - 4);
                        obuf.write_msg_id(5 /* Exception */);
                        obuf.write_msg_type(1 /* Answer */);
                        return;
                    }
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(17);
                    obuf.commit(17);
                    let oa = new Flat_server.server_M16_Direct(obuf, 16);
                    oa._1 = __ret_val;
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 24: {
                    let ia = new Flat_server.server_M18_Direct(buf, 32);
                    let __ret_val /*boolean*/;
                    __ret_val = obj.Notify_DeviceActivated(ia._1);
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(17);
                    obuf.commit(17);
                    let oa = new Flat_server.server_M16_Direct(obuf, 16);
                    oa._1 = __ret_val;
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 25: {
                    let ia = new Flat_server.server_M18_Direct(buf, 32);
                    let __ret_val /*boolean*/;
                    __ret_val = obj.Notify_DeviceDeactivated(ia._1);
                    let obuf = buf;
                    obuf.consume(obuf.size);
                    obuf.prepare(17);
                    obuf.commit(17);
                    let oa = new Flat_server.server_M16_Direct(obuf, 16);
                    oa._1 = __ret_val;
                    obuf.write_len(obuf.size - 4);
                    obuf.write_msg_id(1 /* BlockResponse */);
                    obuf.write_msg_type(1 /* Answer */);
                    break;
                }
                case 26: {
                    let ia = new Flat_server.server_M18_Direct(buf, 32);
                    obj.Notify_ParameterRemoved(ia._1);
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 27: {
                    let ia = new Flat_server.server_M18_Direct(buf, 32);
                    obj.Notify_TypeOrVariableChanged(ia._1);
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 28: {
                    let ia = new Flat_server.server_M18_Direct(buf, 32);
                    obj.History_AddParameter(ia._1);
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                case 29: {
                    let ia = new Flat_server.server_M18_Direct(buf, 32);
                    obj.History_RemoveParameter(ia._1);
                    nprpc.make_simple_answer(buf, 4 /* Success */);
                    break;
                }
                default:
                    nprpc.make_simple_answer(buf, 9 /* Error_UnknownFunctionIdx */);
            }
        }
    }
    function server_throw_exception(buf) {
        switch (buf.read_exception_number()) {
            case 0:
                {
                    let ex_flat = new Flat_server.NPNetCommunicationError_Direct(buf, 16);
                    let ex = new NPNetCommunicationError();
                    ex.code = ex_flat.code;
                    throw ex;
                }
            default:
                throw "unknown rpc exception";
        }
    }
    (function (Flat_server) {
        class server_M1_Direct extends nprpc.Flat.Flat {
            _1(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 0, elements_size, 24, 8);
            }
            _1_vd() { return new nprpc.Flat.Vector_Direct2(this.buffer, this.offset + 0, 24, Flat_server.server_value_Direct); }
        }
        Flat_server.server_M1_Direct = server_M1_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M2_Direct extends nprpc.Flat.Flat {
            get _1() { return new nprpc.detail.Flat_nprpc_base.ObjectId_Direct(this.buffer, this.offset + 0); }
        }
        Flat_server.server_M2_Direct = server_M2_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M3_Direct extends nprpc.Flat.Flat {
            _1(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 0, elements_size, 8, 4);
            }
            _1_vd() { return new nprpc.Flat.Vector_Direct2(this.buffer, this.offset + 0, 8, Flat_server.DataDef_Direct); }
        }
        Flat_server.server_M3_Direct = server_M3_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M4_Direct extends nprpc.Flat.Flat {
            _1(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 0, elements_size, 8, 8);
            }
            _1_vd() { return new nprpc.Flat.Vector_Direct1_u64(this.buffer, this.offset + 0); }
        }
        Flat_server.server_M4_Direct = server_M4_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M5_Direct extends nprpc.Flat.Flat {
            _1(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 0, elements_size, 1, 1);
            }
            _1_vd() { return new nprpc.Flat.Vector_Direct1_u8(this.buffer, this.offset + 0); }
        }
        Flat_server.server_M5_Direct = server_M5_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M6_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get _3() { return this.buffer.dv.getUint8(this.offset + 4); }
            set _3(value) { this.buffer.dv.setUint8(this.offset + 4, value); }
            get _4() { return this.buffer.dv.getUint8(this.offset + 5); }
            set _4(value) { this.buffer.dv.setUint8(this.offset + 5, value); }
        }
        Flat_server.server_M6_Direct = server_M6_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M7_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get _3() { return this.buffer.dv.getUint8(this.offset + 4); }
            set _3(value) { this.buffer.dv.setUint8(this.offset + 4, value); }
            get _4() { return this.buffer.dv.getUint8(this.offset + 5); }
            set _4(value) { this.buffer.dv.setUint8(this.offset + 5, value); }
            get _5() { return this.buffer.dv.getUint8(this.offset + 6); }
            set _5(value) { this.buffer.dv.setUint8(this.offset + 6, value); }
        }
        Flat_server.server_M7_Direct = server_M7_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M8_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get _3() { return this.buffer.dv.getUint8(this.offset + 4); }
            set _3(value) { this.buffer.dv.setUint8(this.offset + 4, value); }
        }
        Flat_server.server_M8_Direct = server_M8_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M9_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get _3() { return this.buffer.dv.getUint16(this.offset + 4, true); }
            set _3(value) { this.buffer.dv.setUint16(this.offset + 4, value, true); }
        }
        Flat_server.server_M9_Direct = server_M9_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M10_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get _3() { return this.buffer.dv.getUint16(this.offset + 4, true); }
            set _3(value) { this.buffer.dv.setUint16(this.offset + 4, value, true); }
            get _4() { return this.buffer.dv.getUint8(this.offset + 6); }
            set _4(value) { this.buffer.dv.setUint8(this.offset + 6, value); }
        }
        Flat_server.server_M10_Direct = server_M10_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M11_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get _3() { return this.buffer.dv.getUint32(this.offset + 4, true); }
            set _3(value) { this.buffer.dv.setUint32(this.offset + 4, value, true); }
        }
        Flat_server.server_M11_Direct = server_M11_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M12_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            get _3() { return this.buffer.dv.getUint32(this.offset + 4, true); }
            set _3(value) { this.buffer.dv.setUint32(this.offset + 4, value, true); }
            get _4() { return this.buffer.dv.getUint8(this.offset + 8); }
            set _4(value) { this.buffer.dv.setUint8(this.offset + 8, value); }
        }
        Flat_server.server_M12_Direct = server_M12_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M13_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
            _3(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 4, elements_size, 1, 1);
            }
            _3_vd() { return new nprpc.Flat.Vector_Direct1_u8(this.buffer, this.offset + 4); }
        }
        Flat_server.server_M13_Direct = server_M13_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M14_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint16(this.offset + 2, true); }
            set _2(value) { this.buffer.dv.setUint16(this.offset + 2, value, true); }
        }
        Flat_server.server_M14_Direct = server_M14_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M15_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
        }
        Flat_server.server_M15_Direct = server_M15_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M16_Direct extends nprpc.Flat.Flat {
            get _1() { return (this.buffer.dv.getUint8(this.offset + 0) === 0x01); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value === true ? 0x01 : 0x00); }
        }
        Flat_server.server_M16_Direct = server_M16_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M17_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getUint8(this.offset + 0); }
            set _1(value) { this.buffer.dv.setUint8(this.offset + 0, value); }
            get _2() { return this.buffer.dv.getUint8(this.offset + 1); }
            set _2(value) { this.buffer.dv.setUint8(this.offset + 1, value); }
            _3(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 4, elements_size, 1, 1);
            }
            _3_vd() { return new nprpc.Flat.Vector_Direct1_u8(this.buffer, this.offset + 4); }
        }
        Flat_server.server_M17_Direct = server_M17_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M18_Direct extends nprpc.Flat.Flat {
            get _1() { return this.buffer.dv.getBigUint64(this.offset + 0, true); }
            set _1(value) { this.buffer.dv.setBigUint64(this.offset + 0, value, true); }
        }
        Flat_server.server_M18_Direct = server_M18_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat 
    (function (Flat_server) {
        class server_M19_Direct extends nprpc.Flat.Flat {
            _1(elements_size) {
                nprpc.Flat._alloc(this.buffer, this.offset + 0, elements_size, 2, 2);
            }
            _1_vd() { return new nprpc.Flat.Vector_Direct1_u16(this.buffer, this.offset + 0); }
        }
        Flat_server.server_M19_Direct = server_M19_Direct;
    })(Flat_server || (Flat_server = {})); // namespace Flat

    const subscriber_queue = [];
    /**
     * Create a `Writable` store that allows both updating and reading by subscription.
     * @param {*=}value initial value
     * @param {StartStopNotifier=}start start and stop notifications for subscriptions
     */
    function writable(value, start = noop) {
        let stop;
        const subscribers = new Set();
        function set(new_value) {
            if (safe_not_equal(value, new_value)) {
                value = new_value;
                if (stop) { // store is ready
                    const run_queue = !subscriber_queue.length;
                    for (const subscriber of subscribers) {
                        subscriber[1]();
                        subscriber_queue.push(subscriber, value);
                    }
                    if (run_queue) {
                        for (let i = 0; i < subscriber_queue.length; i += 2) {
                            subscriber_queue[i][0](subscriber_queue[i + 1]);
                        }
                        subscriber_queue.length = 0;
                    }
                }
            }
        }
        function update(fn) {
            set(fn(value));
        }
        function subscribe(run, invalidate = noop) {
            const subscriber = [run, invalidate];
            subscribers.add(subscriber);
            if (subscribers.size === 1) {
                stop = start(set) || noop;
            }
            run(value);
            return () => {
                subscribers.delete(subscriber);
                if (subscribers.size === 0) {
                    stop();
                    stop = null;
                }
            };
        }
        return { set, update, subscribe };
    }

    let webserver;
    let server;
    let poa;
    let cats = new Array();
    const VQUALITY = 0x00002000;
    const SIGNED = 0x00001000;
    const SIZE_8 = 0x00000001;
    const SIZE_16 = 0x00000002;
    const SIZE_32 = 0x00000004;
    const FLOAT_VALUE = 0x00000010;
    const BIT_VALUE = 0x00000020;
    const INT_VALUE = 0x00000040;
    const TYPE_MASK = BIT_VALUE | INT_VALUE | FLOAT_VALUE | SIGNED | SIZE_8 | SIZE_16 | SIZE_32;
    var VT;
    (function (VT) {
        VT[VT["VT_UNDEFINE"] = 0] = "VT_UNDEFINE";
        VT[VT["VT_DISCRETE"] = (SIZE_8 | BIT_VALUE)] = "VT_DISCRETE";
        VT[VT["VT_BYTE"] = (SIZE_8 | INT_VALUE)] = "VT_BYTE";
        VT[VT["VT_SIGNED_BYTE"] = (SIGNED | SIZE_8 | INT_VALUE)] = "VT_SIGNED_BYTE";
        VT[VT["VT_WORD"] = (SIZE_16 | INT_VALUE)] = "VT_WORD";
        VT[VT["VT_SIGNED_WORD"] = (SIGNED | SIZE_16 | INT_VALUE)] = "VT_SIGNED_WORD";
        VT[VT["VT_DWORD"] = (SIZE_32 | INT_VALUE)] = "VT_DWORD";
        VT[VT["VT_SIGNED_DWORD"] = (SIGNED | SIZE_32 | INT_VALUE)] = "VT_SIGNED_DWORD";
        VT[VT["VT_FLOAT"] = (SIGNED | SIZE_32 | FLOAT_VALUE)] = "VT_FLOAT";
    })(VT || (VT = {}));
    function is_vt_has_quality(type) {
        return type & VQUALITY ? true : false;
    }
    class online_value {
        type_;
        value;
        check_quality_8(data) {
            if (is_vt_has_quality(this.type_) && data.get_val(1) != 0x01)
                return false;
            return true;
        }
        check_quality_16(data) {
            if (is_vt_has_quality(this.type_) && data.get_val(2) != 0x01)
                return false;
            return true;
        }
        check_quality_32(data) {
            if (is_vt_has_quality(this.type_) && data.get_val(4) != 0x01)
                return false;
            return true;
        }
        set_value(val) {
            switch (val.s) {
                case 2 /* VST_UNKNOWN */:
                    this.value.set("unk");
                    return;
                case 1 /* VST_DEVICE_NOT_RESPONDED */:
                    this.value.set("nc");
                    return;
                case 0 /* VST_DEVICE_RESPONDED */:
                    break;
                default:
                    console.error();
            }
            let data = val.data_vd();
            const clear_type = this.type_ & TYPE_MASK;
            switch (clear_type) {
                case VT.VT_DISCRETE:
                    if (!this.check_quality_8(data)) {
                        this.value.set("x");
                        break;
                    }
                    this.value.set((data.data_view.getUint8(0) === 0xFF) ? "1" : "0");
                    break;
                case VT.VT_BYTE:
                case VT.VT_SIGNED_BYTE:
                    if (!this.check_quality_8(data)) {
                        this.value.set("x");
                        break;
                    }
                    if (clear_type == VT.VT_BYTE) {
                        this.value.set(data.data_view.getUint8(0).toString());
                    }
                    else {
                        this.value.set(data.data_view.getInt8(0).toString());
                    }
                    break;
                case VT.VT_WORD:
                case VT.VT_SIGNED_WORD:
                    if (!this.check_quality_16(data)) {
                        this.value.set("x");
                        break;
                    }
                    if (clear_type == VT.VT_WORD) {
                        this.value.set(data.data_view.getUint16(0, true).toString());
                    }
                    else {
                        this.value.set(data.data_view.getInt16(0, true).toString());
                    }
                    break;
                case VT.VT_DWORD:
                case VT.VT_SIGNED_DWORD:
                case VT.VT_FLOAT:
                    if (!this.check_quality_32(data)) {
                        this.value.set("x");
                        break;
                    }
                    switch (clear_type) {
                        case VT.VT_DWORD:
                            this.value.set(data.data_view.getUint32(0, true).toString());
                            break;
                        case VT.VT_SIGNED_DWORD:
                            this.value.set(data.data_view.getInt32(0, true).toString());
                            break;
                        case VT.VT_FLOAT:
                            this.value.set(data.data_view.getFloat32(0, true).toFixed(3));
                            break;
                    }
                    break;
                default:
                    console.error("Unknown type %d", this.type_ & TYPE_MASK);
            }
        }
        constructor(type) {
            this.type_ = type;
            this.value = writable("unk");
        }
    }
    class OnDataCallbackImpl extends _IDataCallBack_Servant {
        items_;
        OnDataChanged(a) {
            for (let x of a) {
                this.items_.get(x.h).ov.set_value(x);
                //console.log(this.items_.get(x.h).ov.value);
            }
            //for (let i of valid_items) {
            //	console.log(i.desc.name + " - " + i.ov.value);
            //}
        }
        constructor() {
            super();
            this.items_ = new Map();
        }
    }
    async function rpc_init() {
        //	NPRPC.set_debug_level(NPRPC.DebugLevel.DebugLevel_EveryCall);
        let rpc = nprpc.init();
        poa = rpc.create_poa(32);
        let nameserver = nprpc.get_nameserver("192.168.1.2");
        {
            let obj = nprpc.make_ref();
            await nameserver.Resolve("npsystem_webserver", obj);
            webserver = nprpc.narrow(obj.value, WebServer);
            if (!webserver)
                throw "WS.WebServer narrowing failed";
        }
        {
            let obj = nprpc.make_ref();
            await nameserver.Resolve("npsystem_server", obj);
            server = nprpc.narrow(obj.value, Server);
            if (!server)
                throw "SRV.Server narrowing failed";
        }
        await webserver.get_web_categories(cats);
        //console.log(cats);
        let item_manager = null;
        {
            let obj = nprpc.make_ref();
            await server.CreateItemManager(obj);
            item_manager = nprpc.narrow(obj.value, ItemManager);
            if (!item_manager)
                throw "SRV.ItemManager narrowing failed";
            item_manager.add_ref();
        }
        let data_callback = new OnDataCallbackImpl();
        let oid = poa.activate_object(data_callback);
        await item_manager.Activate(oid);
        let valid_items = new Array();
        for (let cat of cats) {
            for (let item of cat.items) {
                let ov = new online_value(item.type);
                item.ov = ov;
                if (item.dev_addr != 0xFF)
                    valid_items.push({ desc: item, ov: ov });
            }
        }
        let ar = new Array(valid_items.length);
        for (let i = 0; i < valid_items.length; ++i) {
            ar[i] = {
                dev_addr: valid_items[i].desc.dev_addr,
                mem_addr: valid_items[i].desc.mem_addr,
                type: valid_items[i].desc.type
            };
        }
        let handles = new Array();
        await item_manager.Advise(ar, handles);
        for (let i = 0; i < valid_items.length; ++i) {
            data_callback.items_.set(handles[i], valid_items[i]);
        }
    }

    /* src/App.svelte generated by Svelte v3.42.1 */
    const file = "src/App.svelte";

    function get_each_context(ctx, list, i) {
    	const child_ctx = ctx.slice();
    	child_ctx[0] = list[i];
    	return child_ctx;
    }

    // (8:1) <CollapsibleSection headerText={cat.name} expanded={true}>
    function create_default_slot(ctx) {
    	let category;
    	let t;
    	let current;

    	category = new Category({
    			props: { cat: /*cat*/ ctx[0] },
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(category.$$.fragment);
    			t = space();
    		},
    		m: function mount(target, anchor) {
    			mount_component(category, target, anchor);
    			insert_dev(target, t, anchor);
    			current = true;
    		},
    		p: noop,
    		i: function intro(local) {
    			if (current) return;
    			transition_in(category.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(category.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(category, detaching);
    			if (detaching) detach_dev(t);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_default_slot.name,
    		type: "slot",
    		source: "(8:1) <CollapsibleSection headerText={cat.name} expanded={true}>",
    		ctx
    	});

    	return block;
    }

    // (7:1) {#each cats as cat}
    function create_each_block(ctx) {
    	let collapsiblesection;
    	let current;

    	collapsiblesection = new CollapsibleSection({
    			props: {
    				headerText: /*cat*/ ctx[0].name,
    				expanded: true,
    				$$slots: { default: [create_default_slot] },
    				$$scope: { ctx }
    			},
    			$$inline: true
    		});

    	const block = {
    		c: function create() {
    			create_component(collapsiblesection.$$.fragment);
    		},
    		m: function mount(target, anchor) {
    			mount_component(collapsiblesection, target, anchor);
    			current = true;
    		},
    		p: function update(ctx, dirty) {
    			const collapsiblesection_changes = {};

    			if (dirty & /*$$scope*/ 8) {
    				collapsiblesection_changes.$$scope = { dirty, ctx };
    			}

    			collapsiblesection.$set(collapsiblesection_changes);
    		},
    		i: function intro(local) {
    			if (current) return;
    			transition_in(collapsiblesection.$$.fragment, local);
    			current = true;
    		},
    		o: function outro(local) {
    			transition_out(collapsiblesection.$$.fragment, local);
    			current = false;
    		},
    		d: function destroy(detaching) {
    			destroy_component(collapsiblesection, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_each_block.name,
    		type: "each",
    		source: "(7:1) {#each cats as cat}",
    		ctx
    	});

    	return block;
    }

    function create_fragment(ctx) {
    	let main;
    	let current;
    	let each_value = cats;
    	validate_each_argument(each_value);
    	let each_blocks = [];

    	for (let i = 0; i < each_value.length; i += 1) {
    		each_blocks[i] = create_each_block(get_each_context(ctx, each_value, i));
    	}

    	const out = i => transition_out(each_blocks[i], 1, 1, () => {
    		each_blocks[i] = null;
    	});

    	const block = {
    		c: function create() {
    			main = element("main");

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].c();
    			}

    			attr_dev(main, "class", "svelte-vgipci");
    			add_location(main, file, 5, 0, 164);
    		},
    		l: function claim(nodes) {
    			throw new Error("options.hydrate only works if the component was compiled with the `hydratable: true` option");
    		},
    		m: function mount(target, anchor) {
    			insert_dev(target, main, anchor);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				each_blocks[i].m(main, null);
    			}

    			current = true;
    		},
    		p: function update(ctx, [dirty]) {
    			if (dirty & /*cats*/ 0) {
    				each_value = cats;
    				validate_each_argument(each_value);
    				let i;

    				for (i = 0; i < each_value.length; i += 1) {
    					const child_ctx = get_each_context(ctx, each_value, i);

    					if (each_blocks[i]) {
    						each_blocks[i].p(child_ctx, dirty);
    						transition_in(each_blocks[i], 1);
    					} else {
    						each_blocks[i] = create_each_block(child_ctx);
    						each_blocks[i].c();
    						transition_in(each_blocks[i], 1);
    						each_blocks[i].m(main, null);
    					}
    				}

    				group_outros();

    				for (i = each_value.length; i < each_blocks.length; i += 1) {
    					out(i);
    				}

    				check_outros();
    			}
    		},
    		i: function intro(local) {
    			if (current) return;

    			for (let i = 0; i < each_value.length; i += 1) {
    				transition_in(each_blocks[i]);
    			}

    			current = true;
    		},
    		o: function outro(local) {
    			each_blocks = each_blocks.filter(Boolean);

    			for (let i = 0; i < each_blocks.length; i += 1) {
    				transition_out(each_blocks[i]);
    			}

    			current = false;
    		},
    		d: function destroy(detaching) {
    			if (detaching) detach_dev(main);
    			destroy_each(each_blocks, detaching);
    		}
    	};

    	dispatch_dev("SvelteRegisterBlock", {
    		block,
    		id: create_fragment.name,
    		type: "component",
    		source: "",
    		ctx
    	});

    	return block;
    }

    function instance($$self, $$props, $$invalidate) {
    	let { $$slots: slots = {}, $$scope } = $$props;
    	validate_slots('App', slots, []);
    	const writable_props = [];

    	Object.keys($$props).forEach(key => {
    		if (!~writable_props.indexOf(key) && key.slice(0, 2) !== '$$' && key !== 'slot') console.warn(`<App> was created with unknown prop '${key}'`);
    	});

    	$$self.$capture_state = () => ({ Category, CollapsibleSection, cats });
    	return [];
    }

    class App extends SvelteComponentDev {
    	constructor(options) {
    		super(options);
    		init(this, options, instance, create_fragment, safe_not_equal, {});

    		dispatch_dev("SvelteRegisterComponent", {
    			component: this,
    			tagName: "App",
    			options,
    			id: create_fragment.name
    		});
    	}
    }

    let app;
    rpc_init().then(() => {
        app = new App({
            target: document.body,
            props: {}
        });
    }).catch((e) => console.log(e));
    var app$1 = app;

    return app$1;

}());
//# sourceMappingURL=bundle.js.map
