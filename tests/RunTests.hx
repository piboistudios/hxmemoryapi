package;

import tink.testrunner.*;
import tink.unit.*;
import tink.unit.Assert.assert;
import sys.FileSystem;
import Utils.attempt;
import Utils.shouldFail;
import synch.*;

using haxe.Int64;
using Utils;
using tink.CoreApi;
using Lambda;

class RunTests {
	static function main() {
		// final tests:Array<BasicTest> = [];
		Runner.run(TestBatch.make([new BasicTest()])).handle(Runner.exit);
	}
}

@:asserts
class BasicTest implements tink.unit.Benchmark {
	public function new() {}

	var FILE_SIZE:haxe.Int64 = haxe.Int64.fromFloat(Math.pow(2, 16));
	var mapping:mem.FileMapping;
	var view:mem.MapView;

	public function ensureFile() {
		return assert(attempt({
			var write = sys.io.File.write('./test.db');
			var written:haxe.Int64 = 0;
			while (written < FILE_SIZE) {
				written += Std.int(Math.pow(2, 13));
				write.write(haxe.io.Bytes.alloc(Std.int(Math.pow(2, 13))));
			}
			write.flush();
			write.close();
		}));
	}

	public function testCreateFileMapping() {
		return assert(attempt({
			mapping = mem.FileMapping.create("./test.db", "test-map", FILE_SIZE);
		}));
	}

	public function testCreateFileView() {
		return assert(attempt({
			view = mapping.createView(0, mem.MemLib.get_sys_granularity());
		}));
	}

	public function testWriteToView() {
		return assert(attempt({
			view.write(haxe.io.Bytes.ofString("some data"), 0);
		}));
	}

	public function testPersistToView() {
		return assert(attempt({
			view.persist();
		}));
	}

	public function testReadView() {
		return assert(attempt(view.read(0).toString() == 'some data'));
	}

	var currentPage = 0;
	var ppp = 7;

	var pageSize = Std.int(Math.pow(2, 13));
	// @:exclude

	@:benchmark(1)
	public function testConcurrentWrite() {
		// view = mapping.createView(FILE_SIZE/4, mem.MemLib.get_sys_granularity());
		final content = sys.io.File.read('./lorem.txt').read(pageSize - 32);
		var counter = 0;
		final page = currentPage;
		for (i in page...page + ppp) {
			currentPage++;
			final offset = haxe.Int64.fromFloat((i : Float) * pageSize);
			sys.thread.Thread.create(() -> {
				try {
					view.write(content, offset);
					counter++;
				} catch (ex:Dynamic) {
					trace('Couldn\'t write: $ex');
				}
			});
		}
		while (counter < ppp) {
			Sys.sleep(1 / 60);
		}
		// view.persist();
	}

	public function persist() {
		view.persist();
		return assert(true, 'view persisted');
	}
	public function testConcurrentRead() {
		final pages = new Array<haxe.io.Bytes>();
		pages.resize(currentPage);
		final expectedContent = sys.io.File.read('./lorem.txt').read(pageSize - 32);
		var counter = 0;
		for(i in 0...currentPage) {
			final offset = haxe.Int64.fromFloat((i:Float) * pageSize);
			sys.thread.Thread.create(() -> {
				try {
					pages[i] = view.read(offset);
					counter++;
				} catch(ex:Dynamic) {
					trace('Couldn\'t read: $ex');
				}
			});
		}
		while(counter < currentPage) {
			Sys.sleep(1/60);
		}
		for(page in pages) {
			trace(page.toString());
			asserts.assert(page.compare(expectedContent) == 0); 
		}
		return asserts.done();
	}
}
