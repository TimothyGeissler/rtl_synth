module mux2 (
    input a,
    input b,
    input sel,
    output y
);

    // Ternary operator test: y = sel ? b : a
    assign y = sel ? b : a;

endmodule
