// Can put text inside the doughnut chart
export const text = {
	beforeDraw: function(chart) {
        var width = chart.chartArea.width,
        height = chart.chartArea.height,
        ctx = chart.ctx;

        ctx.restore();
        var fontSize = (height / 114).toFixed(2);
        ctx.font = fontSize + "em sans-serif";
        ctx.textBaseline = "middle";

        var text = chart.data.datasets[0].data.reduce((partialSum, a) => partialSum + a, 0) + "Go",
        textX = Math.round((width - ctx.measureText(text).width) / 2),
        textY = (height / 2) + chart.legend.height + chart.titleBlock.height;

        ctx.fillText(text, textX, textY);
        ctx.save();
    }
};


//Show Text In Center Onhover in Doughnut Chart
export const hoverLabel = {
	id: 'hoverLabel',
	afterDraw(chart, args, options) {
		const {ctx, chartArea: {left, right, top, bottom, width, height}} = chart;
		ctx.save();

        if(chart._active.length > 0) {

            const textLabel = chart.config.data.labels[chart._active[0].index];
            const numberLabel = chart.config.data.datasets[chart._active[0].datasetIndex].data[chart._active[0].index];
            const nb_lettres = (textLabel + numberLabel).length+1+2;

            var fontSize = (((height / 114)/(0.2*nb_lettres)).toFixed(2));
            ctx.font = fontSize + "em Helvetica";
            ctx.textBaseline = "middle";
    
            ctx.fillStyle = 'rgba(0, 0, 0, 1)';
            var text = chart.data.datasets[0].data.reduce((partialSum, a) => partialSum + a, 0),
            textX = Math.round((width ) / 2),
            textY = (height / 2) + chart.legend.height + chart.titleBlock.height;
            ctx.textAlign = 'center';
            ctx.fillText(`${textLabel}: ${numberLabel} Go`, textX, textY);
            
        }
        ctx.restore();
	},
}