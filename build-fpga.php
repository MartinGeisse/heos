#!/usr/bin/env php
<?php

define('TARGET', 'fpga');
define('TOOL', '~/riscv-toolchain/bin/riscv32-unknown-linux-gnu-');

function buildFileForTarget($inputPath, $outputPath, $extension) {
    $allowedExtensions = array('c', 'cpp', 'S');
    if (!in_array($extension, $allowedExtensions)) {
        die('unknown input file extension: ' . $extension);
    }

    $cppFlags = ($extension == '.cpp' ? ' -fno-rtti ' : '');
    $baseCommand = TOOL . 'gcc -msmall-data-limit=100000 -march=rv32im -mabi=ilp32 -fno-exceptions ' . $cppFlags .
        ' -Wall -Wextra -O3 -fno-tree-loop-distribute-patterns -I../bootloader/exported';
    if ($extension != 'S') {
        system($baseCommand . ' -S  -o ' . $outputPath . '.S ' . $inputPath);
    }
    system($baseCommand . ' -c  -o ' . $outputPath . ' ' . $inputPath);
}

function linkFiles() {
    global $objectFiles;
    $objectFilesList = implode(' ', $objectFiles);
    system(TOOL . 'ld -T buildscript/linkerscript-fpga -Map=out-fpga/program.map -A rv32im -o out-fpga/program.elf ' . $objectFilesList);
}

function convertExecutable() {
    system(TOOL . 'objcopy -j .image -I elf32-littleriscv -O binary out-fpga/program.elf out-fpga/program.bin');
}

require('buildscript/common.php');

//
// --------------------------------------------------------------------------------------------------------------------
//

prepareOutputFolder();
buildDirectory('src', 'out-fpga');
linkFiles();
convertExecutable();
