package mem;

class FileMapping {
    public static function create(filename:String, mapname:String, mapSize:haxe.Int64):mem.FileMapping {
        throw 'not implemented';
    }
    public function createView(offset:UInt, size:haxe.Int64):mem.MapView {
        throw 'not implemented';
    }
}