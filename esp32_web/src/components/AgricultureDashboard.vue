<!--
 * @Author: ckdfs 2459317008@qq.com
 * @Date: 2024-05-27 18:06:25
 * @LastEditors: ckdfs 2459317008@qq.com
 * @LastEditTime: 2024-05-27 18:17:31
 * @FilePath: /nong_new/esp32_web/AgricultureDashboard.vue
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->
<template>
  <el-container>
    <el-header>
      <h1>农业大屏</h1>
    </el-header>
    <el-main>
      <el-row>
        <el-col :span="12">
          <div class="grid-content bg-purple">
            <h2>温度</h2>
            <div id="temperature-chart"></div>
          </div>
        </el-col>
        <el-col :span="12">
          <div class="grid-content bg-purple-light">
            <h2>湿度</h2>
            <div id="humidity-chart"></div>
          </div>
        </el-col>
      </el-row>
      <el-row>
        <el-col :span="12">
          <div class="grid-content bg-purple">
            <h2>二氧化碳</h2>
            <div id="co2-chart"></div>
          </div>
        </el-col>
        <el-col :span="12">
          <div class="grid-content bg-purple-light">
            <h2>光照强度</h2>
            <div id="lux-chart"></div>
          </div>
        </el-col>
      </el-row>
    </el-main>
  </el-container>
</template>

<script>
import { use } from "echarts/core";
import { LineChart } from "echarts/charts";
import {
  TitleComponent,
  TooltipComponent,
  GridComponent
} from "echarts/components";
import { CanvasRenderer } from "echarts/renderers";

use([
  TitleComponent,
  TooltipComponent,
  GridComponent,
  LineChart,
  CanvasRenderer
]);

export default {
  mounted() {
    // 在这里初始化你的图表，并使用你的数据更新图表
  }
};
</script>

<style scoped>
.el-header {
  background-color: #b3c0d1;
  color: #333;
  line-height: 60px;
}

.el-main {
  background-color: #e9eef3;
  color: #333;
}

.bg-purple {
  background: #d3adf7;
}

.bg-purple-light {
  background: #d3adf7;
}

.grid-content {
  border-radius: 4px;
  min-height: 36px;
}
</style>