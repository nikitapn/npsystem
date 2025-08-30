// Setup WebSocket polyfill for Node.js BEFORE importing NPRPC
import './websocket-polyfill';

import { describe, it, before, after } from 'mocha';
import { expect } from 'chai';
// Use the Node.js compatible build
// const NPRPC = require('nprpc/index.node.js');
import * as NPRPC from 'nprpc';
import * as test from '../src/gen/test';
import { ServerManager } from './server-manager';

// Test data constants (matching C++ tests)
const nested_test_str1 = "34k535n3jknjknfdjgndfgnj4kjn545kr3k4n3k45j n34k kgfdg334r34rnkfndkgn[34l5k32;45l324lk;5j324k5j32lk4;5j34lknmt3l2k4ng54;nt295t0-2jt2ithnjkgnsfdgmndf;gkj3m450934j5234k5n345n345lkjn643kj6n4kljnkbvnvcb456456456gfhfgh56y";
const nested_test_str2 = "d34234k24j1;lk24j12lk341234n45n345n3kj45n2345jn34lk5jn34kj5n3lk5jn2354lk3j2n4532-49503245i32k4095i34rjjngdfn;dfgns;kgfsd;lfkgsd;lgfkmgo4523[459j324509345i302945hjnkjngdgmsfg,msdb'sdf'3;4l5k32[59j2345igfgdsgfdg34345345bbdfgdsgdgdsfgdfvcbxcvbdfg34trjg";

describe('NPRPC Integration Tests', function() {
    this.timeout(30000); // 30 second timeout for all tests
    
    const serverManager = new ServerManager();
    let rpc: NPRPC.Rpc;
    let nameserver: NPRPC.Nameserver;

    before(async function() {
        this.timeout(60000); // 60 seconds for setup
        
        console.log('Setting up NPRPC test environment...');
        
        // Start nameserver and test server
        const serversStarted = await serverManager.startAll();
        if (!serversStarted) {
            throw new Error('Failed to start servers');
        }

        // Initialize RPC connection
        try {
            rpc = await NPRPC.init(false);
            console.log('RPC initialized successfully');
        } catch (error) {
            serverManager.stopAll();
            throw new Error(`Failed to initialize RPC: ${error}`);
        }

        // Get nameserver proxy
        try {
            nameserver = NPRPC.get_nameserver('127.0.0.1');
            console.log('Nameserver proxy created');
            
            // Wait a bit more for test server to register objects
            await new Promise(resolve => setTimeout(resolve, 3000));
        } catch (error) {
            serverManager.stopAll();
            throw new Error(`Failed to get nameserver: ${error}`);
        }

        console.log('NPRPC test environment setup complete');
    });

    after(function() {
        console.log('Cleaning up NPRPC test environment...');
        serverManager.stopAll();
    });

    async function resolveTestObject<T extends NPRPC.ObjectProxy>(objectName: string, obj: new() => T): Promise<any> {
        try {
            const objRef = NPRPC.make_ref<NPRPC.ObjectProxy>();
            const found = await nameserver.Resolve(objectName, objRef);
            
            if (!found) {
                throw new Error(`Failed to resolve object: ${objectName}`);
            }

            const narrowedObj = NPRPC.narrow(objRef.value, obj);
            if (!narrowedObj) {
                throw new Error(`Failed to narrow object to desired type: ${objectName}`);
            }

            return narrowedObj;
        } catch (error) {
            throw new Error(`Failed to resolve test object ${objectName}: ${error}`);
        }
    }

    describe('TestBasic Interface', function() {
        let testBasic: test.TestBasic;

        before(async function() {
            testBasic = await resolveTestObject('nprpc_test_basic', test.TestBasic);
        });

        it('should return boolean true', async function() {
            const result = await testBasic.ReturnBoolean();
            expect(result).to.be.true;
        });

        it('should handle input parameters correctly', async function() {
            // Create test data matching C++ test
            const testData = new Array(256);
            for (let i = 0; i < 256; i++) {
                testData[i] = i;
            }

            const result = await testBasic.In(100, true, new Uint8Array(testData));
            expect(result).to.be.true;
        });

        it('should handle output parameters correctly', async function() {
            // Create ref objects for output parameters
            const aRef = NPRPC.make_ref();
            const bRef = NPRPC.make_ref();
            const cArray = NPRPC.make_ref<Uint8Array>();
            
            // Call the Out method
            await testBasic.Out(aRef, bRef, cArray);
            
            // Verify output values match C++ implementation
            expect(aRef.value).to.equal(100);
            expect(bRef.value).to.be.true;
            expect(cArray.value.length).to.equal(256);
            
            // Verify the array content (should be 0, 1, 2, ..., 255)
            for (let i = 0; i < 256; i++) {
                expect(cArray.value[i]).to.equal(i);
            }
        });

        it('should return array of IDs as Array<number>', async function() {
            const result = await testBasic.ReturnIdArray();
            expect(result).to.be.an('array').that.has.lengthOf(10);
            expect(result).to.be.an('array').that.deep.equals([1,2,3,4,5,6,7,8,9,10]);
        });
    });

    describe('TestOptional Interface', function() {
        let testOptional: test.TestOptional;

        before(async function() {
            testOptional = await resolveTestObject('nprpc_test_optional', test.TestOptional);
        });

        it('should handle empty optional values', async function() {
            const result = await testOptional.InEmpty(); // No parameter = empty optional
            expect(result).to.be.true;
        });

        it('should handle optional values with data', async function() {
            // Create test AAA object
            const testAAA = {
                a: 100,
                b: "test_b",
                c: "test_c"
            };
            const result = await testOptional.In(100, testAAA);
            expect(result).to.be.true;
        });

        it('should return empty optional values', async function() {
            // OutEmpty and Out methods don't return values, they are void methods
            // The return data would be through optional output parameters if they existed
            try {
                const a = NPRPC.make_ref<number>();
                await testOptional.OutEmpty(a);
                expect(a.value).to.be.undefined; // Should remain undefined for empty optional
            } catch (error) {
                console.error('OutEmpty call failed:', error);
                throw error;
            }
        });

        it('should return optional values with data', async function() {
            try {
                const a = NPRPC.make_ref<number>();
                await testOptional.Out(a);
                expect(a.value).to.equal(100); // Should be set by server
            } catch (error) {
                console.error('Out call failed:', error);
                throw error;
            }
        });
        
        it('should return optional', async function() {
            try {
                const opt = await testOptional.ReturnOpt1();
                expect(opt.str).to.equal('test_string');
                expect(opt.stream).to.be.an('uint8array').that.has.lengthOf(10);
                expect(opt.stream).to.deep.equal(new Uint8Array([0,1,2,3,4,5,6,7,8,9]));
            } catch (error) {
                console.error('Out call failed:', error);
                throw error;
            }
        });
    });

    describe('TestNested Interface', function() {
        let testNested: test.TestNested;
        before(async function() {
            testNested = await resolveTestObject('nprpc_test_nested', test.TestNested);
        });

        it('should handle nested structures correctly', async function() {
            try {
                const reference = NPRPC.make_ref<test.BBB>();
                await testNested.Out(reference);

                expect(reference.value).to.not.be.undefined;
                const value = reference.value;
                expect(value.a).to.be.an('array').that.has.lengthOf(1024);
                for (let i = 0; i < 1024; i++) {
                    expect(value.a[i].a).to.equal(i);
                    expect(value.a[i].b).to.equal(nested_test_str1);
                    expect(value.a[i].c).to.equal(nested_test_str2);
                }
                expect(value.b).to.be.an('array').that.has.lengthOf(2048);
                let b = false;
                for (let i = 0; i < 2048; i++) {
                    expect(value.b[i].a).to.equal(nested_test_str1);
                    expect(value.b[i].b).to.equal(nested_test_str2);
                    expect(value.b[i].c).to.not.be.undefined;
                    expect(value.b[i].c).to.equal(b = !b);
                }
            } catch (error) {
                console.error('Out call failed:', error);
                throw error;
            }
        });

        it('should handle deeply nested structures correctly', async function() {
            try {
                const result = await testNested.ReturnNested();
                expect(result.x).to.equal('top_level_string');
                expect(result.z).to.equal(1n);
                expect(result.y).to.not.be.undefined;
                expect(result.y.x).to.equal('level1_string');
                expect(result.y.z).to.equal(2n);
                expect(result.y.y).to.not.be.undefined;
                expect(result.y.y.x).to.equal('level2_string');
                expect(result.y.y.z).to.equal(3n);
                expect(result.y.y.y).to.be.an('uint8array').that.has.lengthOf(10);
                expect(result.y.y.y).to.deep.equal(new Uint8Array([1,2,3,4,5,6,7,8,9,10]));
            } catch (error) {
                console.error('ReturnNested call failed:', error);
                throw error;
            }
        });
    });
});
