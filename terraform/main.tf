provider "aws" {
  region = "eu-west-1"
}

resource "aws_vpc" "poker" {
  cidr_block = "10.0.0.0/16"
}

resource "aws_internet_gateway" "poker" {
  vpc_id = aws_vpc.poker.id
}

resource "aws_route_table" "poker" {
  vpc_id = aws_vpc.poker.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.poker.id
  }

  route {
    ipv6_cidr_block = "::/0"
    gateway_id      = aws_internet_gateway.poker.id
  }
}

resource "aws_subnet" "poker" {
  vpc_id            = aws_vpc.poker.id
  cidr_block        = "10.0.1.0/24"
  availability_zone = "eu-west-1a"
}

resource "aws_route_table_association" "poker" {
  subnet_id      = aws_subnet.poker.id
  route_table_id = aws_route_table.poker.id
}

resource "aws_security_group" "poker_trainer" {
  name        = "poker_trainer"
  description = "Allow poker trainer traffic"
  vpc_id      = aws_vpc.poker.id

  ingress {
    description = "SSH"
    from_port   = 22
    to_port     = 22
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }
}

resource "aws_instance" "poker_trainer" {
  ami                         = "ami-0694d931cee176e7d"
  instance_type               = "t3.micro"
  availability_zone           = "eu-west-1a"
  key_name                    = "main-key"
  associate_public_ip_address = true
  subnet_id                   = aws_subnet.poker.id
  security_groups             = [aws_security_group.poker_trainer.id]
}

resource "aws_ebs_volume" "poker_trainer_data" {
  availability_zone = "eu-west-1a"
  size              = 10
}

resource "aws_volume_attachment" "poker_trainer_data" {
  device_name = "/dev/sdb"
  volume_id   = aws_ebs_volume.poker_trainer_data.id
  instance_id = aws_instance.poker_trainer.id
}
