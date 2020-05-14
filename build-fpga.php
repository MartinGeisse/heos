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
    if ($extension == 'h') {
        return;
    }
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

function buildDirectory($inputPath, $outputPath) {
    $filenames = scandir($inputPath);
    sort($filenames);
    foreach ($filenames as $filename) {
        if ($filename == '.' || $filename == '..') {
            continue;
        }
        $dotPosition = strrpos($filename, '.');
        if ($dotPosition === FALSE) {
            $inputFolder = $inputPath . '/' . $filename;
            $outputFolder = $outputPath . '/' . $filename;
            system('mkdir ' . $outputFolder);
            buildDirectory($inputFolder, $outputFolder);
        } else {
            $baseName = substr($filename, 0, $dotPosition);
            $inputFile = $inputPath . '/' . $filename;
            $outputFile = $outputPath . '/' . $baseName . '.o';
            echo $inputFile, ' -> ', $outputFile, "\n";
            buildFile($inputFile, $outputFile);
        }
    }
}

function linkFiles() {
    global $objectFiles;
    $objectFilesList = implode(' ', $objectFiles);
    system(TOOL . 'ld -T src/linkerscript -Map=out-fpga/program.map -A rv32im -o out-fpga/program.elf ' . $objectFilesList);
}

function convertExecutable() {
    system(TOOL . 'objcopy -j .image -I elf32-littleriscv -O binary out-fpga/program.elf out-fpga/program.bin');
}

//
// --------------------------------------------------------------------------------------------------------------------
//

system('rm -rf out-fpga');
system('mkdir out-fpga');

buildDirectory('src', 'out-fpga');
linkFiles();
convertExecutable();
