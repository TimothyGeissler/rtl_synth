// 2-stage shift register using two D flip-flops
// q1 captures din on clk rising edge; q2 captures previous q1

module shift2 (
    input clk,
    input din,
    output q1,
    output q2
);
    reg q1_reg, q2_reg;
    assign q1 = q1_reg;
    assign q2 = q2_reg;

    always @(posedge clk) begin
        q1_reg <= din;
        q2_reg <= q1_reg;
    end
endmodule


