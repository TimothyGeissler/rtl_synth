// Testbench for 4-bit Ripple Carry Adder
// This testbench tests various scenarios including edge cases

`timescale 1ns/1ps

module adder4b_tb;

    // Testbench signals
    reg [3:0] a, b;     // Input operands
    reg cin;            // Carry input
    wire [3:0] sum;     // Sum output
    wire cout;          // Carry output
    
    // Expected results for verification
    reg [4:0] expected_result;  // 5-bit to hold carry + 4-bit sum
    reg [4:0] actual_result;    // 5-bit to hold carry + 4-bit sum
    
    // Instantiate the Device Under Test (DUT)
    adder_4bit dut (
        .a(a),
        .b(b),
        .cin(cin),
        .sum(sum),
        .cout(cout)
    );
    
    // Test stimulus and verification
    initial begin
        $display("Starting 4-bit Adder Testbench");
        $display("Time\tA\tB\tCin\tSum\tCout\tExpected\tStatus");
        $display("----------------------------------------------------------------");
        
        // Test Case 1: Basic addition without carry
        a = 4'b0000; b = 4'b0000; cin = 1'b0;
        #10;
        expected_result = 5'b00000;  // 0 + 0 + 0 = 0
        actual_result = {cout, sum};
        check_result("Basic 0+0");
        
        // Test Case 2: Simple addition
        a = 4'b0001; b = 4'b0001; cin = 1'b0;
        #10;
        expected_result = 5'b00010;  // 1 + 1 + 0 = 2
        actual_result = {cout, sum};
        check_result("Simple 1+1");
        
        // Test Case 3: Addition with carry input
        a = 4'b0001; b = 4'b0001; cin = 1'b1;
        #10;
        expected_result = 5'b00011;  // 1 + 1 + 1 = 3
        actual_result = {cout, sum};
        check_result("With carry 1+1+1");
        
        // Test Case 4: Maximum values without overflow
        a = 4'b0111; b = 4'b0001; cin = 1'b0;
        #10;
        expected_result = 5'b01000;  // 7 + 1 + 0 = 8
        actual_result = {cout, sum};
        check_result("Max-1 + 1");
        
        // Test Case 5: Overflow case
        a = 4'b1111; b = 4'b0001; cin = 1'b0;
        #10;
        expected_result = 5'b10000;  // 15 + 1 + 0 = 16 (overflow)
        actual_result = {cout, sum};
        check_result("Overflow 15+1");
        
        // Test Case 6: Maximum overflow
        a = 4'b1111; b = 4'b1111; cin = 1'b1;
        #10;
        expected_result = 5'b11111;  // 15 + 15 + 1 = 31
        actual_result = {cout, sum};
        check_result("Max overflow 15+15+1");
        
        // Test Case 7: Random test cases
        a = 4'b1010; b = 4'b0101; cin = 1'b0;  // 10 + 5 = 15
        #10;
        expected_result = 5'b01111;
        actual_result = {cout, sum};
        check_result("Random 10+5");
        
        a = 4'b1100; b = 4'b0011; cin = 1'b1;  // 12 + 3 + 1 = 16
        #10;
        expected_result = 5'b10000;
        actual_result = {cout, sum};
        check_result("Random 12+3+1");
        
        // Test Case 8: Edge cases with different bit patterns
        a = 4'b1010; b = 4'b0101; cin = 1'b0;  // 10 + 5 = 15
        #10;
        expected_result = 5'b01111;
        actual_result = {cout, sum};
        check_result("Pattern 1010+0101");
        
        a = 4'b1111; b = 4'b0000; cin = 1'b1;  // 15 + 0 + 1 = 16
        #10;
        expected_result = 5'b10000;
        actual_result = {cout, sum};
        check_result("Pattern 1111+0000+1");
        
        $display("----------------------------------------------------------------");
        $display("Testbench completed!");
        $finish;
    end
    
    // Task to check and display results
    task check_result;
        input [80:0] test_name;
        begin
            $write("%0t\t%h\t%h\t%b\t%h\t%b\t%b\t", 
                   $time, a, b, cin, sum, cout, expected_result);
            
            if (actual_result == expected_result) begin
                $display("PASS");
            end else begin
                $display("FAIL - Expected: %b, Got: %b", expected_result, actual_result);
            end
        end
    endtask
    
    // Generate VCD file for waveform viewing
    initial begin
        $dumpfile("adder4b_tb.vcd");
        $dumpvars(0, adder4b_tb);
    end

endmodule

