#!/usr/bin/perl
use strict;
use warnings;
use JSON;
use Text::CSV;

my $task = $ARGV[0];
my $experiment = $ARGV[1];

my $write_path = '/home/ashton/tmp/cache_test_write';
my $write_path2 = '/home/ashton/tmp/cache_test_write2';

my $read_path = '/home/ashton/tmp/cache_test_read';
my $read_path2 = '/home/ashton/tmp/cache_test_read2';


if (scalar @ARGV == 0) {
    print("No input provided \n");
    print("Please select \"setup\", \"run\", or \"clean\"\n");
    exit();
}

chomp($task);
chomp($experiment);

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
        my $num_runs = 5;  # Number of iterations to run each test
        $graphCMD = 'plot_both';
        $x_column = 'Access Size';
        @x_data = (8*1024, 16*1024, 32*1024, 64*1024, 128*1024);

        for my $i (0 .. $#x_data) {
            my ($read_lat_sum, $read_speed_sum, $write_lat_sum, $write_speed_sum) = (0, 0, 0, 0);

            for (1 .. $num_runs) {  # Run the test $num_runs times
                # Run read test
                system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=randread --size=$full_size --bs=$x_data[$i] --iodepth=16");
                my $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $read_lat_sum += $job->{'read'}->{'lat_ns'}->{'mean'};
                    $read_speed_sum += $job->{'read'}->{'io_bytes'} / $job->{'read'}->{'runtime'} / 1024 / 1024 * 1000;
                }

                # Run write test
                system("fio test.fio --output-format=json --output=output.json --filename=$write_path --rw=randwrite --size=$full_size --bs=$x_data[$i] --iodepth=16");
                $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $write_lat_sum += $job->{'write'}->{'lat_ns'}->{'mean'};
                    $write_speed_sum += $job->{'write'}->{'io_bytes'} / $job->{'write'}->{'runtime'} / 1024 / 1024 * 1000;
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
        my $num_runs = 3;  # Number of iterations to run each test
        @x_data = (.0, .25, .50, .75, 1.0);
        $graphCMD = 'combine_both';
        $x_column = 'Read Percentage';

        for my $i (0 .. $#x_data) {
            my $read_cent = $x_data[$i];
            my $write_cent = 1 - $read_cent;
            my ($read_lat_sum, $read_speed_sum, $write_lat_sum, $write_speed_sum) = (0, 0, 0, 0);

            for (1 .. $num_runs) {
                # Run read test
                system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=randread --size=1GiB --bs=16MiB --iodepth=16");
                my $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $read_lat_sum += $job->{'read'}->{'lat_ns'}->{'mean'} * $read_cent;
                    $read_speed_sum += $job->{'read'}->{'io_bytes'} / $job->{'read'}->{'runtime'} / 1024 / 1024 * 1000 * $read_cent;
                }

                # Run write test
                system("fio test.fio --output-format=json --output=output.json --filename=$write_path --rw=randwrite --size=1GiB --bs=16MiB --iodepth=16");
                $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $write_lat_sum += $job->{'write'}->{'lat_ns'}->{'mean'} * $write_cent;
                    $write_speed_sum += $job->{'write'}->{'io_bytes'} / $job->{'write'}->{'runtime'} / 1024 / 1024 * 1000 * $write_cent;
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
        my $num_runs = 8    ;  # Number of iterations to run each test
        @x_data = (1, 2, 4, 8, 16, 64, 128);
        $graphCMD = 'combine_both';
        $x_column = "Queue Depth";

        for my $i (0 .. $#x_data) {
            my ($read_lat_sum, $read_speed_sum, $write_lat_sum, $write_speed_sum) = (0, 0, 0, 0);

            for (1 .. $num_runs) {
                # Run read test
                system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=randread --size=$half_size --bs=8MiB --iodepth=$x_data[$i]");
                my $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $read_lat_sum += $job->{'read'}->{'lat_ns'}->{'mean'} * 0.5;
                    $read_speed_sum += $job->{'read'}->{'io_bytes'} / $job->{'read'}->{'runtime'} / 1024 / 1024 * 1000 * 0.5;
                }

                # Run write test
                system("fio test.fio --output-format=json --output=output.json --filename=$read_path --rw=randwrite --size=$half_size --bs=8MiB --iodepth=$x_data[$i]");
                $data = decode_json_file('output.json');

                foreach my $job (@{$data->{'jobs'}}) {
                    $write_lat_sum += $job->{'write'}->{'lat_ns'}->{'mean'} * 0.5;
                    $write_speed_sum += $job->{'write'}->{'io_bytes'} / $job->{'write'}->{'runtime'} / 1024 / 1024 * 1000 * 0.5;
                }
            }

            # Calculate and store the averages
            push(@read_lat, $read_lat_sum / $num_runs);
            push(@read_speed, $read_speed_sum / $num_runs);
            push(@write_lat, $write_lat_sum / $num_runs);
            push(@write_speed, $write_speed_sum / $num_runs);
        }
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

    system("python3 graph.py $graphCMD");
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
