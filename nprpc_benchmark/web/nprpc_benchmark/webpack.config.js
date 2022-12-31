const path = require('path');

module.exports = env => {
	let is_debug = env.debug == 'true' ? true : false;
  return {
	entry: './src/index.ts',
	watch: is_debug,
	mode: is_debug ? 'development' : 'production',
  module: {
    rules: [
      {
        test: /\.tsx?$/,
        use: 'ts-loader',
        exclude: /node_modules/,
      },
    ],
  },
  resolve: {
    modules: [
      path.resolve(__dirname + '/src'),
      path.resolve(__dirname + '/node_modules')
    ],
    extensions: ['.ts', '.js'],
  },
  output: {
    filename: 'index.js',
    path: path.resolve(__dirname, './public/build'),
  },
}
};