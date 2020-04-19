
# GPSR routing agent settings
for {set i 0} {$i < $opt(nn)} {incr i} {
    $ns_ at 0.00002 "$ragent_($i) turnon"
    $ns_ at 20.0 "$ragent_($i) neighborlist"
#    $ns_ at 30.0 "$ragent_($i) turnoff"
}

$ns_ at 11.0 "$ragent_(0) startSink 10.0"
$ns_ at 10.5 "$ragent_(99) startSink 10.0"


# GPSR routing agent dumps
$ns_ at 25.0 "$ragent_(24) sinklist"


# Upper layer agents/applications behavior
set null_(1) [new Agent/Null]
$ns_ attach-agent $node_(0) $null_(1)

set udp_(1) [new Agent/UDP]
$ns_ attach-agent $node_(98) $udp_(1)

set cbr_(1) [new Application/Traffic/CBR]
$cbr_(1) set packetSize_ 32
$cbr_(1) set interval_ 2.0
$cbr_(1) set random_ 1
#    $cbr_(1) set maxpkts_ 100
$cbr_(1) attach-agent $udp_(1)
$ns_ connect $udp_(1) $null_(1)
$ns_ at 76.0 "$cbr_(1) start"
$ns_ at 150.0 "$cbr_(1) stop" 


set udp_(2) [new Agent/UDP]
$ns_ attach-agent $node_(89) $udp_(2)

set null_(2) [new Agent/Null]
$ns_ attach-agent $node_(99) $null_(2)

set cbr_(2) [new Application/Traffic/CBR]
$cbr_(2) set packetSize_ 32
$cbr_(2) set interval_ 2.0
$cbr_(2) set random_ 1
#    $cbr_(2) set maxpkts_ 2
$cbr_(2) attach-agent $udp_(2)
$ns_ connect $udp_(2) $null_(2)
$ns_ at 76.0 "$cbr_(2) start"
$ns_ at 150.0 "$cbr_(2) stop" 



set udp_(3) [new Agent/UDP]
$ns_ attach-agent $node_(51) $udp_(3)

set null_(3) [new Agent/Null]
$ns_ attach-agent $node_(99) $null_(3)

set cbr_(3) [new Application/Traffic/CBR]
$cbr_(3) set packetSize_ 32
$cbr_(3) set interval_ 2.0
$cbr_(3) set random_ 1
#    $cbr_(3) set maxpkts_ 2
$cbr_(3) attach-agent $udp_(3)
$ns_ connect $udp_(3) $null_(3)
$ns_ at 76.0 "$cbr_(3) start"
$ns_ at 150.0 "$cbr_(3) stop" 
