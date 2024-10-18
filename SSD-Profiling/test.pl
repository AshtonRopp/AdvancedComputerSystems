#!/usr/bin/perl
use strict;
use warnings;
use JSON;
use Text::CSV;

my $task = $ARGV[0];
my $experiment = $ARGV[1];

my $count = 1024;   # 1 GiB = 1024 MiB
my $path = '/mnt/c/tmp/cache_test';

if (scalar @ARGV == 0) {
    print("No input provided \n");
    print("Please select \"setup\", \"run\", or \"clean\"\n");
    exit()
}

chomp($task);
chomp($experiment);

if ($task eq 'setup') {
    # MiB = 1024*1024 B
    system("dd if=/dev/random of=$path bs=MiB count=$count");
}

elsif ($task eq 'run') {
    my @read_IOPs;
    my @read_speed;
    my @write_IOPs;
    my @write_speed;
    my @x_data;
    my $graphCMD;
    my $x_column; # Header for x data column in csv

    if ($experiment eq 'data_access') {
        $graphCMD = 'plot_both';
        $x_column = "Access-Sizes";
        my $half_size = int(1024*1024*1024/2);
        @x_data = (4*1024, 16*1024, 32*1024, 128*1024);
        for (my $i = 0; $i < 4; $i++) {
            system("fio test.fio --output-format=json --output=output.json --filename=$path --rw=randread --size=$half_size --bs=$x_data[$i]");

            # File path to the fio JSON output
            my $file = 'output.json';

            # Read the JSON file
            open my $fh, '<', $file or die "Could not open file '$file' $!";
            my $json_text = do { local $/; <$fh> };
            close $fh;

            # Decode the JSON into a Perl data structure
            my $data = decode_json($json_text);

            # Iterate through the jobs in the fio output
            foreach my $job (@{$data->{'jobs'}}) {
                # Extract read IOPS and bandwidth (in bytes)
                push(@read_IOPs, $job->{'read'}->{'iops'});
                push(@read_speed, $job->{'read'}->{'io_bytes'}/$job->{'read'}->{'runtime'}/1024/1024*1000);
            }

            system("fio test.fio --output-format=json --output=output.json --filename=$path --rw=randwrite --size=$half_size --bs=$x_data[$i]");

            # Read the JSON file
            open  $fh, '<', $file or die "Could not open file '$file' $!";
            $json_text = do { local $/; <$fh> };
            close $fh;

            # Decode the JSON into a Perl data structure
            $data = decode_json($json_text);

            # Iterate through the jobs in the fio output
            foreach my $job (@{$data->{'jobs'}}) {
                # Extract read IOPS and bandwidth (in bytes)
                push(@write_IOPs, $job->{'write'}->{'iops'});
                push(@write_speed, $job->{'write'}->{'io_bytes'}/$job->{'write'}->{'runtime'}/1024/1024*1000);
            }
        }

    }

    elsif ($experiment eq 'rw_cent') {
        my $full_size = 1024*1024*1024; # 1 GIB
        @x_data = (.0, .10, .25, .50, 1.0);
        $graphCMD = 'combine_both';
        $x_column = "Read-Percentage";

        for (my $i = 0; $i < 4; $i++) {
            my $read_cent = $x_data[$i];
            my $write_cent = 1-$read_cent;
            my $read_size = int($read_cent*$full_size);
            my $write_size = int($write_cent*$full_size);

            # Need to filter out if read/write percent = 0
            system("fio test.fio --output-format=json --output=output.json --filename=$path --rw=randread --size=$read_size --bs=16MiB");

            # File path to the fio JSON output
            my $file = 'output.json';

            # Read the JSON file
            open my $fh, '<', $file or die "Could not open file '$file' $!";
            my $json_text = do { local $/; <$fh> };
            close $fh;

            # Decode the JSON into a Perl data structure
            my $data = decode_json($json_text);

            # Iterate through the jobs in the fio output
            foreach my $job (@{$data->{'jobs'}}) {
                # Extract read IOPS and bandwidth (in bytes)
                push(@read_IOPs, $job->{'read'}->{'iops'}*$read_cent);
                push(@read_speed, $job->{'read'}->{'io_bytes'}/$job->{'read'}->{'runtime'}/1024/1024*1000*$read_cent);
            }

            system("fio test.fio --output-format=json --output=output.json --filename=$path --rw=randwrite --size=$write_size --bs=16MiB");

            # Read the JSON file
            open  $fh, '<', $file or die "Could not open file '$file' $!";
            $json_text = do { local $/; <$fh> };
            close $fh;

            # Decode the JSON into a Perl data structure
            $data = decode_json($json_text);

            # Iterate through the jobs in the fio output
            foreach my $job (@{$data->{'jobs'}}) {
                # Extract read IOPS and bandwidth (in bytes)
                push(@write_IOPs, $job->{'write'}->{'iops'}*$write_cent);
                push(@write_speed, $job->{'write'}->{'io_bytes'}/$job->{'write'}->{'runtime'}/1024/1024*1000*$write_cent);
            }
        }

    }
    # Create a new CSV object
    my $csv = Text::CSV->new({ binary => 1, auto_diag => 1 });

    # Open a CSV file for writing
    open my $fh, '>', 'data.csv' or die "Cannot open 'data.csv' for write: $!";

    # Write header
    $csv->print($fh, [$x_column, 'Read Speed', 'Write Speed', 'Read IOPs', 'Write IOPs']);
    print $fh "\n";  # Add a newline after the header

    # Write data rows
    for my $i (0 .. $#x_data) {
        $csv->print($fh, [$x_data[$i], $read_speed[$i], $write_speed[$i], $read_IOPs[$i], $write_IOPs[$i]]);
        print $fh "\n";  # Add a newline to separate lines
    }

    close $fh;

    print "Data exported to data.csv\n";

    system("python3 graph.py $graphCMD, $x_column");
}

elsif ($task eq 'clean'){
    system("rm $path");
}
