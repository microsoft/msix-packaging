// we output msix.node in build/bin/msix.node, so binding cannot find it.
// var msix = require('bindings')('msix');
const msix = require("./build/bin/msix.node");

console.log(msix.unpack(0, 0, "src\\test\\testData\\unpack\\StoreSigned_Desktop_x64_MoviesTV.appx", "node_output"));