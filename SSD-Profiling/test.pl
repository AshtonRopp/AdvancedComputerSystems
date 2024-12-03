#!/usr/bin/perl
use strict;
use warnings;
use JSON;
use Text::CSV;

my $task = $ARGV[0];
my $experiment = $ARGV[1];
my $input_runs = $ARGV[2];

# Edit for your system:
my $write_path = '/home/ashton/tmp/cache_test_write';
my $read_path = '/home/ashton/tmp/cache_test_read';
# End editing

if (scalar @ARGV == 0) {
    die "Please select \"setup\", \"run\", or \"clean\"\n";
}

foreach my $arg (@ARGV) {
    if (defined $arg && $arg ne '') {
        chomp($arg) if $arg =~ /\R$/;
    }
}

my $n = 2; # Number of GiB
my $full_size = $n*1024*1024*1024; # n GiB
my $half_size = $full_size/2;      # n/2 GiB
my $count = $full_size/1024/1024; # Number of MiB in full_size

if ($task eq 'setup') {
    # MiB = 1024*1024 B
    system("dd if=/dev/random of=$write_path bs=MiB count=$count");
    system("dd if=/dev/random of=$read_path bs=MiB count=$count");
}

elsif ($task eq 'run') {
    my @read_lat;    # Read latency (ns)
    my @read_speed;  # Read speed (MiB/s)
    my @write_lat;   # Write latency (ns)
    my @write_speed; # Read speed (MiB/s)
    my @x_data;      # Stores data to plot on x-axis
    my $x_column;    # Header for x data column in CSV
    my $graphCMD;    # Type of graph to create

    if ($experiment eq 'data_access') {
        my $num_runs = 40;  # Number of iterations to run each test

        # Use input number of runs if specified
        if(@ARGV == 3) {
            $num_runs = $input_runs;
        }

        $graphCMD = 'plot_both';
        $x_column = 'Access Size';
        @x_data = (16*1024, 32*1024, 64*1024, 128*1024);

        for my $i (0 .. $#x_data) {
            my ($read_lat_sum, $read_speed_sum, $write_lat_sum, $write_speed_sum) = (0, 0, 0, 0);

            for (1 .. $num_runs) {  # Run the test $num_runs times
                # Run read test
                system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=randread --size=$full_size --bs=$x_data[$i] --iodepth=32 --runtime=3 --time_based");
                my $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $read_lat_sum += $job->{'read'}->{'lat_ns'}->{'mean'};
                    $read_speed_sum += $job->{'read'}->{'bw_mean'} / 1024;
                }

                # Run write test
                system("fio test.fio --output-format=json --output=output.json --filename=$write_path --rw=randwrite --size=$full_size --bs=$x_data[$i] --iodepth=32 --runtime=3 --time_based");
                $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $write_lat_sum += $job->{'write'}->{'lat_ns'}->{'mean'};
                    $write_speed_sum += $job->{'write'}->{'bw_mean'} / 1024;
                }
            }

            # Calculate and store the averages
            push(@read_lat, $read_lat_sum / $num_runs);
            push(@read_speed, $read_speed_sum / $num_runs);
            push(@write_lat, $write_lat_sum / $num_runs);
            push(@write_speed, $write_speed_sum / $num_runs);
        }
    }

    elsif ($experiment eq 'rw_cent') {
        my $num_runs = 1;  # Number of iterations to run each test

        # Use input number of runs if specified
        if(@ARGV == 3) {
            $num_runs = $input_runs;
        }

        @x_data = (.0, .25, .50, .75, 1.0);
        $graphCMD = 'combine_rw';
        $x_column = 'Read Percentage';

        for my $i (0 .. $#x_data) {
            my $read_cent = $x_data[$i];
            my $write_cent = 1.0 - $read_cent;
            my ($read_lat_sum, $read_speed_sum, $write_lat_sum, $write_speed_sum) = (0, 0, 0, 0);

            for (1 .. $num_runs) {
                # Run read test
                system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=randread --size=$half_size --bs=64KiB --iodepth=32 --runtime=3 --time_based");
                my $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $read_lat_sum += $job->{'read'}->{'lat_ns'}->{'mean'} * $read_cent;
                    $read_speed_sum += $job->{'read'}->{'bw'} / 1024 * $read_cent;
                }

                # Run write test
                system("fio test.fio --output-format=json --output=output.json --filename=$write_path --rw=randwrite --size=$half_size --bs=64KiB --iodepth=32 --runtime=3 --time_based");
                $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $write_lat_sum += $job->{'write'}->{'lat_ns'}->{'mean'} * $write_cent;
                    $write_speed_sum += $job->{'write'}->{'bw'} / 1024 * $write_cent;
                }
            }

            # Calculate and store the averages
            push(@read_lat, $read_lat_sum / $num_runs);
            push(@read_speed, $read_speed_sum / $num_runs);
            push(@write_lat, $write_lat_sum / $num_runs);
            push(@write_speed, $write_speed_sum / $num_runs);
        }
    }

    elsif ($experiment eq 'queue_depth') {
        my $num_runs = 5;  # Number of iterations to run each test

        # Use input number of runs if specified
        if(@ARGV == 3) {
            $num_runs = $input_runs;
        }

        @x_data = (1, 4, 16, 64, 256, 1024);
        $graphCMD = 'plot_both';
        $x_column = "Queue Depth";

        for my $i (0 .. $#x_data) {
            my ($read_lat_sum, $read_speed_sum, $write_lat_sum, $write_speed_sum) = (0, 0, 0, 0);

            for (1 .. $num_runs) {
                # Run read test
                system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=randread --size=$half_size --bs=64KiB --iodepth=$x_data[$i]");
                my $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $read_lat_sum += $job->{'read'}->{'lat_ns'}->{'mean'};
                    $read_speed_sum += $job->{'read'}->{'bw_mean'} / 1024;
                }

                # Run write test
                system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=randwrite --size=$half_size --bs=64KiB --iodepth=$x_data[$i]");
                $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $write_lat_sum += $job->{'write'}->{'lat_ns'}->{'mean'};
                    $write_speed_sum += $job->{'write'}->{'bw_mean'} / 1024;
                }
            }

            # Calculate and store the averages
            push(@read_lat, $read_lat_sum / $num_runs);
            push(@read_speed, $read_speed_sum / $num_runs);
            push(@write_lat, $write_lat_sum / $num_runs);
            push(@write_speed, $write_speed_sum / $num_runs);
        }
    }

    elsif ($experiment eq 'max_perfomance') {
        system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=read --size=$full_size --bs=16384KiB --iodepth=128 --runtime=30 --time_based");
        my $data = decode_json_file('output.json');
        my $lat = 0;
        my $bw = 0;

        foreach my $job (@{$data->{'jobs'}}) {
            $lat += $job->{'read'}->{'lat_ns'}->{'mean'}/1000;
            $bw += $job->{'read'}->{'bw_mean'} / 1024;
        }

        # Convert from Mebibyte to Megabyte
        $bw *= 1.04858;
        print "\nLatency: ", $lat/1000000, " ms\n";
        print 'Bandwidth: ', $bw, " MB/s\n";
        exit();
    }


    # Write results to CSV
    my $csv = Text::CSV->new({ binary => 1, auto_diag => 1 });
    open my $fh, '>', 'data.csv' or die "Cannot open 'data.csv' for write: $!";

    $csv->print($fh, [$x_column, 'Read Speed', 'Write Speed', 'Read Latency', 'Write Latency']);
    print $fh "\n";

    for my $i (0 .. $#x_data) {
        $csv->print($fh, [$x_data[$i], $read_speed[$i], $write_speed[$i], $read_lat[$i], $write_lat[$i]]);
        print $fh "\n";
    }

    close $fh;

    print "Data exported to data.csv\n";

    # Edit for your system
    system("python3 graph.py $graphCMD");
    # End edit
}

elsif ($task eq 'clean') {
    system("rm $write_path");
    system("rm $read_path");
}

# Helper function to decode JSON from a file
sub decode_json_file {
    my ($file) = @_;
    open my $fh, '<', $file or die "Could not open file '$file' $!";
    my $json_text = do { local $/; <$fh> };
    close $fh;
    return decode_json($json_text);
}
