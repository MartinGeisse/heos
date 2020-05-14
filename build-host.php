#!/usr/bin/env php
<?php

define('TOOL', '~/riscv-toolchain/bin/riscv32-unknown-linux-gnu-');

$objectFiles = array();

function buildFile($inputPath, $outputPath) {
    global $objectFiles;
    $objectFiles[] = $outputPath;

    $dotPosition = strrpos($inputPath, '.');
    if ($dotPosition === FALSE) {
        die('no dot in input filename');
    }
    $extension = substr($inputPath, $dotPosition + 1);
    $allowedExtensions = array('c', 'cpp', 'S');
    if (!in_array($extension, $allowedExtensions)) {
        die('unknown input file extension: ' . $extension);
    }

    $cppFlags = ($extension == '.cpp' ? ' -fno-rtti ' : '');
    $baseCommand = TOOL . 'gcc -msmall-data-limit=100000 -march=rv32im -mabi=ilp32 -fno-exceptions ' . $cppFlags .
        ' -Wall -Wextra -O2 -std=gnu99 -fno-tree-loop-distribute-patterns -I../bootloader/exported';
    if ($extension != 'S') {
        system($baseCommand . ' -S  -o ' . $outputPath . '.S ' . $inputPath);
    }
    system($baseCommand . ' -c  -o ' . $outputPath . ' ' . $inputPath);
}

function linkFiles() {
    global $objectFiles;
    $objectFilesList = implode(' ', $objectFiles);
    system(TOOL . 'ld -T src/linkerscript -Map=out-host/program.map -A rv32im -o out-host/program.elf ' . $objectFilesList);
}

function convertExecutable() {
    system(TOOL . 'objcopy -j .image -I elf32-littleriscv -O binary out-host/program.elf out-host/program.bin');
}

//
// --------------------------------------------------------------------------------------------------------------------
//

system('rm -rf out-host');
system('mkdir out-host');

$paths = array('src/start.S' => 'out-host/start.o');
$filenames = scandir('src');
$filenames = array('loadlib.c');
foreach ($filenames as $filename) {
    if (strpos($filename, '.c') === strlen($filename) - 2) {
        $paths['src/' . $filename] = 'out-host/' . substr($filename, 0, strlen($filename) - 2) . '.o';
    }
}

foreach ($paths as $inputPath => $outputPath) {
    buildFile($inputPath, $outputPath);
}
linkFiles();
convertExecutable();
