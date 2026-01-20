use std.io;
use std.file;
use std.padding;

var replace_space = func(s) {
    var res = "";
    for (var i = 0; i < size(s); i += 1) {
        var c = chr(s[i]);
        if (c == ` ` or c == `\n` or c == `\r` or c == `\t`) {
            continue;
        }
        res ~= c;
    }
    return res;
}

var count_lines = func(dir_path, file_list) {
    println("<", dir_path, ">:");
    var total = 0;
    var empty_line_count = 0;
    foreach (var f; file_list) {
        var content = io.readfile(f);
        var lines = split("\n", content);
        var line_count = size(lines);
        foreach (var line; lines) {
            if (size(replace_space(line)) == 0) {
                empty_line_count += 1;
            }
        }
        total += line_count;
        println("  ", padding.leftpad((line_count), 6), "  ", f);
    }
    println("   total  ", total);
    println("   empty  ", empty_line_count);
}

var find_files = file.recursive_find_files_with_extension;
count_lines("bootstrap", find_files("bootstrap", "h", "cpp", "hpp"));
count_lines("src", find_files("src", "colgm"));
