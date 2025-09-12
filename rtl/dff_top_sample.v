// Top-level using behavioral DFF pattern the converter understands

module dff_top (
    input clk,
    input d,
    output q
);
    // Behavioral DFF pattern
    reg q_reg;
    assign q = q_reg;
    always @(posedge clk) begin
        q_reg <= d;
    end
endmodule


