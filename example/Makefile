# Copyright (c) 2017 Sony Global Education, Inc.

JSON=crocodile_bite_ir.json crocodile_bite.json \
	koov_helicopter.json koov_isoginchaku.json koov_loco.json \
	koov_s_armcar_touchsensor.json koov_whale_shark_aqa.json \
	koov_yadokari.json servo-motor.json koov_dog.json koov_monkey.json \
	koov_guiter.json accelerometer.json button.json \
	koov_s_armcar_touchsensor_0806.json koov_asahigani.json \
	crocodile_bite_0809.json digital-sensor.json buzzer-oneshot.json \
	timer.json list.json all-blocks.json math.json

all: $(JSON)

%.json: %.scm block.scm
	(cd ..; gosh example/$<) | jq . > $@

crocodile_bite_ir.json: crocodile_bite.scm block.scm
	(cd ..; gosh example/crocodile_bite.scm --ir) | jq . > $@

