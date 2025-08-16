---
layout: post
title: Tự động xử lý các dữ liệu forensic
date: 2025-08-16
subtitle:
tags:
- forensic
--- 
- [Tại sao cần tự động phân tích các dữ liệu forensic?](#tại-sao-cần-tự-động-phân-tích-các-dữ-liệu-forensic)
  - [Kape](#kape)
  - [log2timeline/plaso](#log2timelineplaso)
  - [siftgrab](#siftgrab)
- [dfir\_auto\_parser](#dfir_auto_parser)

*New episode every third Saturday, 2025-09-20*

# Tại sao cần tự động phân tích các dữ liệu forensic?

TL;DR: [Wrapper auto chạy các parser xử lý dữ liệu forensics của windows](https://github.com/hongson11698/dfir_auto_parser)

Khi thu thập và thực hiện rà soát trên 1 máy chủ đơn lẻ, hướng tiếp cận cơ bản sẽ là sử dụng các công cụ để đọc và kiểm tra từng artifact đã thu thập trên máy chủ như evtx, mft, registry,.. Việc này sẽ tốn nhiều thời gian và không còn phù hợp đối với các trường hợp cần rà soát nhiều máy chủ khác nhau liên quan.

Do đó mình thấy cần có cách để tự động xử lý các dữ liệu đã thu thập, trước hết là phân tích các dữ liệu ra dạng data như csv, json để có thể rà soát cho một hoặc nhiều máy chủ một cách nhanh chóng hơn. Mục tiêu đặt ra là:
- Tự động tìm, phân tích các dữ liệu forensics
- Hỗ trợ chọn các dữ liệu cần rà soát để xử lý nhanh hơn và mở rộng điều tra khi cần thiết.
- Có thể tự động xử lý dữ liệu của một hoặc nhiều máy chủ
- Có thể tích hợp trong quá trình tự động xử lý dữ liệu sau khi phân tích
  
Sau một thời gian tìm kiếm, mình thấy có một số công cụ có thể setup và maintain được tương đối đơn giản để sử dụng trong việc phân tích dữ liệu và cho phép chọn các module dữ liệu cần phân tích nhưng khả năng để xử lý cho nhiều máy chủ thì còn hạn chế và đều cần tạo script để hỗ trợ hoặc tùy biến thêm.

## [Kape](https://ericzimmerman.github.io/KapeDocs/) 

Thường được biết đến và ứng dụng nhiều trong việc thu thập dữ liệu, bên cạnh đó Kape còn có khả năng chạy các module để xử lý các dữ liệu đã thu thập.

![alt text](..\assets\2025-08-16-automated-parsing-artifact\kape.png)

Bản thân kape cũng giống như một wrapper chuyên chạy các công cụ phân tích dữ liệu khác do đó chỉ cần thêm số bước là có thể sử dụng kape tự động như mục tiêu đã đặt ra:
- Viết hoặc sử dụng lại các file .mkape đã có và tạo ra các module tổng hợp riêng để chọn các parser cần dùng.
- Viết script batch hoặc powershell,.. để tự động tìm và đến các thư mục chứa log và chạy kape với command line phù hợp.

## [log2timeline/plaso](https://plaso.readthedocs.io/en/latest/)

Bộ công cụ python khá lâu đời để tạo timeline từ nhiều dữ liệu forensics khác nhau. Khi sử dụng log2timeline mình thấy có một số điểm sau:
- log2timeline cho phép tự động tìm và parse dữ liệu sang dạng event timeline, nhưng sẽ xử lý toàn bộ dữ liệu từ input. Do đó cần phải dùng image_export để trích xuất lại các artifact muốn dùng trước khi thự hiện tạo timeline.
- Công cụ tương đối chậm do cần xử lý và đồng bộ để tạo ra timeline hoàn chỉnh. 
 
![alt text](..\assets\2025-08-16-automated-parsing-artifact\plaso.png)

Bộ công cụ có một số khía cạnh phù hợp với mục tiêu nhưng mình cảm thấy chưa phù hợp hoàn toàn và cần can thiệp nhiều mới có thể thỏa mãn được.

## [siftgrab](https://github.com/dfir-scripts/siftgrab)
Một bash script tổng hợp nhiều công cụ phân tích xử lý các dữ liệu forensics khác nhau. Không rõ phát triển từ bao giờ nhưng gần đây mình mới phát hiện ra nó trong khi tìm cách setup môi trường cho script tự viết :)). 
- Có [file setup](https://raw.githubusercontent.com/dfir-scripts/installers/main/install-forensic-tools.sh) để thực hiện cài các công cụ phân tích. 
- Hỗ trợ tự động phân tích dữ liệu thu thập từ 1 hoặc nhiều hệ thống
- Có nhiều tùy chọn để xử lý các dữ liệu khác. 

![alt text](..\assets\2025-08-16-automated-parsing-artifact\siftgrab.png)

Có lẽ đây là công cụ gần giống với định hướng ban đầu của mình nhất. Lúc mới tìm thấy mình đã định drop việc tự build script khác nhưng khi thử ứng dụng siftgrab thì gặp một vài vấn đề:
- Là công cụ AIO nên khi muốn tùy biến các parser cần chạy thì cần can thiệp nhiều vào script chính của công cụ.
- Công cụ định hướng dạng trực quan interactive qua các menu chọn nên muốn dùng tự động trong luồng xử lý khác cũng cần chỉnh sửa khá nhiều.

# [dfir_auto_parser](https://github.com/hongson11698/dfir_auto_parser)
Mỗi công cụ trên có ưu/nhược điểm khác nhau đối với mong muốn ban đầu của mình, do đó sau cùng thì mình vẫn quyết định tự build 1 cái script cho dễ tùy biến và mong là nó có ích với những ai có cùng nhu cầu. 

![alt text](..\assets\2025-08-16-automated-parsing-artifact\dfir_auto_parser_help.png)

Công cụ có một số đặc điểm:
- Là một dạng wrapper tích hợp nhiều công cụ khác nhau, có thể tùy chọn các parser cần chạy. 
- Hiện mới thêm những parser cơ bản và phổ biến. Đa số các artifact sẽ sử dụng tool của [Eric Zimmerman](https://ericzimmerman.github.io/) để phân tích do mình cảm thấy các công cụ này chạy nhanh hơn các script python/perl,.. dùng trong forensics và hỗ trợ cả Linux thông qua .NET.
- Script sẽ chạy trên linux để có thể tích hợp vào luồng xử lý dữ liệu và không phải suy nghĩ về vấn đề bản quyền trên windows. Hơn nữa nếu bỏ thời gian để tùy biến kape thì mình nghĩ sẽ hiệu quả hơn khi muốn phân tích trên windows host. Luồng xử lý dữ liệu sau khi phân tích có thể sẽ bao gồm các bước:
    - Xử lý output của parser ra các định dạng dữ liệu chuẩn (các trường, cột, thuộc tính,...)
    - Tự động rà soát và gán thêm các thuộc tính về dấu hiệu nghi ngờ đối với từng loại dữ liệu
    - Import dữ liệu lên các hệ thống như ELK/Splunk để phân tích

Hy vọng sắp tới mình tìm được cách chuẩn hóa các dữ liệu khi thu thập trên linux để có thể làm script tự động phân tích tương tự như đối với các artifact của windows và hoạn thiện các công cụ trong luồng xử lý dữ liệu forensic trên cả 2 nền tảng windows và linux.

![alt text](..\assets\2025-08-16-automated-parsing-artifact\forensic_analyzing.png)

